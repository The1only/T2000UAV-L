#pragma once
#include <string>
#include <vector>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

enum class SensorMode : int {
    RADAR = 1,
    IMU = 2,
    TRANSPONDER = 3,
};

SensorMode get_sensor_mode();
const char* sensor_mode_name(SensorMode m);

// Telnet broadcast API
void telnet_start();
void telnet_broadcast(const uint8_t* data, size_t len);

// SSDP responder
void ssdp_start();

// Wi-Fi
void wifi_start();

// Serial console (UART0) commands
void serial_console_start();
