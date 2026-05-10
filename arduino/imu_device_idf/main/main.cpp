#include <cstdio>
#include "esp_log.h"
#include "nvs_flash.h"
#include "main.hpp"

/*
START_DIR=$(pwd) 
cd ~/esp/esp-idf-v5.1 
source ./export.sh 
cd "$START_DIR"
#
idf.py set-target esp32s3
idf.py menuconfig
idf.py build
idf.py flash monitor
#
Close the terminal tab/window
Press Ctrl + C (might need to press it multiple times)
Use the menu: Ctrl + T then Ctrl + X to exit
*/

static const char* TAG = "app";

SensorMode get_sensor_mode() {
#if CONFIG_SENSOR_MODE == 1
    return SensorMode::RADAR;
#elif CONFIG_SENSOR_MODE == 2
    return SensorMode::IMU;
#else
    return SensorMode::TRANSPONDER;
#endif
}

const char* sensor_mode_name(SensorMode m) {
    switch (m) {
        case SensorMode::RADAR: return "RADAR";
        case SensorMode::IMU: return "IMU";
        case SensorMode::TRANSPONDER: return "TRANSPONDER";
        default: return "UNKNOWN";
    }
}

extern "C" void app_main(void) {
    ESP_LOGI(TAG, "Booting imu_device_idf (mode=%s)", sensor_mode_name(get_sensor_mode()));

    // NVS is required for Wi-Fi and credential storage
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    } else {
        ESP_ERROR_CHECK(err);
    }

    wifi_start();
    telnet_start();
    ssdp_start();
    serial_console_start();

#if CONFIG_ENABLE_SIMULATOR
    extern void wit_simulator_start();
    wit_simulator_start();
#endif
}
