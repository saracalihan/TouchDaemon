# TouchDaemon Panel

Tauri + React + TypeScript tabanlı kontrol paneli.

## Gereksinimler

- [Rust](https://www.rust-lang.org/)
- [Node.js](https://nodejs.org/)
- Driver'ın çalışıyor olması (port 8081)

## Geliştirme

```bash
npm install
npm run tauri dev
```

## Build

```bash
npm run tauri build
```

## İletişim Protokolü

Panel, driver ile TCP üzerinden (localhost:8081) haberleşir.

**Server → Panel** — 3-byte envelope:

| type   | isim          | açıklama                        |
|--------|---------------|---------------------------------|
| `0x01` | MSG_TOUCH     | 27-byte touch slot frame        |
| `0x02` | MSG_SHELL_OUT | Shell komutunun çıktı chunk'ı   |
| `0x04` | MSG_SHELL_END | Shell komutunun tamamlandığı    |

**Panel → Server** — `[controller(1)][size(4)][value]` formatında metin:

| controller | açıklama   |
|------------|------------|
| `1`        | Mouse      |
| `2`        | Keyboard   |
| `3`        | Shell      |
| `4`        | DLL        |
| `5`        | D-Bus      |

**Mouse event'leri** (controller=1):

| event | kod | value                  |
|-------|-----|------------------------|
| press     | `1` | `[event][button]` — button: 1=sol, 2=sağ |
| release   | `2` | `[event][button]`      |
| click     | `3` | `[event][button]`      |
| move      | `4` | `[event][mode][x],[y]` — mode: 1=relative, 2=absolute (0-32767) |
| get_info  | `6` | `[event]` — yanıt `tcp-shell-out` ile gelir |

## Tauri Event'leri

| event           | yön             | açıklama                    |
|-----------------|-----------------|-----------------------------|
| `tcp-connected` | backend → UI    | Bağlantı kuruldu            |
| `tcp-data`      | backend → UI    | Touch frame (27-byte str)   |
| `tcp-shell-out` | backend → UI    | Shell çıktı satırı          |
| `tcp-shell-end` | backend → UI    | Shell komutu tamamlandı     |
| `exec-command`  | UI → backend    | Driver'a komut gönder       |
| `tcp-error`     | backend → UI    | Bağlantı hatası             |
