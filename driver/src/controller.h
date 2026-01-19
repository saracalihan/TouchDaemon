#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <dlfcn.h>
#include "common.h"

#define DEVICE_NAME_PREFIX "as-touchpad"
#define DLL_PATHS "/home/saracalihan/Desktop/projects/touchpad/driver/build/libhello.so"

#define MAX_DLL_PATH_LEN 256

typedef struct {
    char path[MAX_DLL_PATH_LEN];
    void* handle;
} Dll;

typedef struct {
    Dll* items;
    int count;
    int capacity;
} Dlls;

typedef enum {
    CT_MOUSE=1,
    CT_KEYBOARD,
    CT_SHELL,
    CT_DLL,
} ControllerType;

typedef enum {
    CT_PRESS=1,
    CT_RELEASE,
    CT_CLICK,
    CT_MOVE,
    CT_EXEC,
} ControllerEvent;

typedef enum {
    MOUSE_LEFT=1,
    MOUSE_RIGTH
} MouseKey;

void init_controllers();
void cleanup_controllers();

void mouse_press(int code);
void mouse_release(int code);
void mouse_click(int code);

void key_press(int code);
void key_release(int code);
void key_click(int code);

int exec_command(ControllerCommand c);
int exec_str_command(char* c);

#endif // CONTROLLER_H