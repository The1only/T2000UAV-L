# imu_device_idf (ESP-IDF)

This is an ESP-IDF rework of your Arduino `.ino` sketches.

It provides:
- Wi‑Fi connection (stored credentials in NVS + fallback AP list)
- Telnet server on port 23 (broadcasts WIT-style packets to all connected clients)
- SSDP/UPnP-style discovery responder over UDP (simple M-SEARCH reply)
- Optional IMU packet simulator task

## Open in VS Code (ESP-IDF extension)
1. Install Espressif **ESP-IDF** extension
2. `File -> Open Folder...` and select this project folder
3. Set target (e.g. ESP32-S3): `ESP-IDF: Set Espressif Device Target`
4. Configure: `ESP-IDF: SDK Configuration Editor` (optional)
5. Build/Flash/Monitor from the ESP-IDF status bar

## Configuration
Open `menuconfig`:
- `Component config -> Project settings -> Sensor mode`
- `Component config -> Project settings -> Enable simulator`

## Wi‑Fi credentials
At boot the firmware:
1. Tries credentials stored in NVS (namespace: `wifi`, keys: `ssid`, `pass`)
2. Falls back to the compiled-in AP list in `main/wifi_manager.cpp`

You can set credentials from the serial console:
- `wifi set <ssid> <pass>`
- `wifi clear`
- `wifi show`

(These commands are intentionally simple; feel free to extend.)

## Notes / what was changed vs Arduino
- `WiFiMulti/WiFiServer/WiFiUDP` replaced with ESP-IDF Wi‑Fi + lwIP sockets
- `Serial` replaced with a UART console reader task
- `delay()/millis()` replaced with FreeRTOS delays + `esp_timer_get_time()`

If you want me to preserve **exact** SSDP payload strings or your full transponder/radar UART framing,
tell me which target board + which UART pins you use, and which sketch is the “source of truth”.
