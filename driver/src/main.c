#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>

#include "common.h"
#include "tcp_server.h"
#include "touchpad_listener.h"
#include "controller.h"
#include "macro_engine.h"

// cat /proc/bus/input/devices
#define TOUCHPAD_DEVICE_NAME "FTCS1000:01 2808:0222 Touchpad"

void cleanup(void) {
    running = false;
    printf("cleanup starting...\n");
    cleanup_macro_engine();
    cleanup_tcp_server();
    cleanup_controllers();
    cleanup_touchpad();
    cleanup_event_buffer();
    printf("cleanup done\n");
}

void signal_handler(int signo) {
    printf("\nReceived signal %d, shutting down...\n", signo);
    cleanup();
    printf("cleanup done\n");
    exit(0);
}

char* find_touchpad_device() {
    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    char *event_path = NULL;
    bool found_device = false;

    fp = fopen("/proc/bus/input/devices", "r");
    if (fp == NULL) {
        perror("Failed to open /proc/bus/input/devices");
        return NULL;
    }

    while ((read = getline(&line, &len, fp)) != -1) {
        if (strstr(line, "N: Name=\"") != NULL && strstr(line, TOUCHPAD_DEVICE_NAME) != NULL) {
            found_device = true;
        }
        if (found_device && strstr(line, "H: Handlers=") != NULL) {
            char *event = strstr(line, "event");
            if (event != NULL) {
                char event_num[3];
                int i = 0;
                while (event[5+i] >= '0' && event[5+i] <= '9' && i < 2) {
                    event_num[i] = event[5+i];
                    i++;
                }
                event_num[i] = '\0';

                event_path = malloc(20);
                snprintf(event_path, 20, "/dev/input/event%s", event_num);
                break;
            }
        }
    }

    free(line);
    fclose(fp);
    return event_path;
}

int main(void) {
    char *dev = find_touchpad_device();
    if (dev == NULL) {
        fprintf(stderr, "Could not find touchpad device\n");
        exit(EXIT_FAILURE);
    }
    pthread_t touchpad_thread;

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    init_event_buffer();
    init_tcp_server();
    init_controllers();
    init_macro_engine();
    init_touchpad(dev);

    free(dev); // Free the allocated device path


    if (pthread_create(&touchpad_thread, NULL, touchpad_event_thread, NULL) != 0) {
        perror("pthread_create");
        cleanup();
        exit(EXIT_FAILURE);
    }

    printf("Event listening and TCP server started. Touchpad events will be continuously monitored.\n");

    while (running) {
        if(get_client() != -1){
            sleep(1);
            continue;
        }
        struct sockaddr_in cli_addr;
        socklen_t sin_size = sizeof(cli_addr);
        int new_client_fd = accept(get_server_fd(), (struct sockaddr *)&cli_addr, &sin_size);

        if (new_client_fd == -1) {
            if (errno == EINTR) continue;
            perror("accept");
            continue;
        }

        printf("New connection: %s:%d\n",
               inet_ntoa(cli_addr.sin_addr),
               ntohs(cli_addr.sin_port));

        set_client(new_client_fd);
        printf("Client connected and set as active\n");
    }

    pthread_join(touchpad_thread, NULL);

    cleanup();
    return 0;
}
