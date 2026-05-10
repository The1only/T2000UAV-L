/**
 * @file imu_device.cpp
 * @brief ESP32 Telnet bridge + SSDP (UPnP) device discovery + optional IMU simulator.
 *
 * It also provides:
 *  - USB serial command interface for setting Wi-Fi credentials
 *
 */

#include <math.h>
#define SIM false;

#define ESP32C3

#ifdef ESP32C3
#define USE_OLED
#endif

#define SENSOR_TYPE "Airplane-device"  ///< SSDP search target (ST)
#define USE_SERIAL true    ///< Use UART bridge instead of IMU simulation

bool SIMULATE = SIM;

// -----------------------------------------------------------------------------
// Timing / LED
// -----------------------------------------------------------------------------
static const uint16_t PERIOD_MS = 20;  ///< Loop delay when simulating IMU (~50 Hz)

String serialLine;  ///< Input buffer for USB-serial commands

// -----------------------------------------------------------------------------
// SSDP / UPnP Discovery
// -----------------------------------------------------------------------------

bool active = true;  //false;

/**
 * @brief Arduino setup. Initializes Wi-Fi, SSDP, Telnet, USART (optional).
 */
void setup() {
  Serial.begin(115200);

  uint32_t start = millis();
  while (!Serial && millis() - start < 5000) {
    delay(10);
  }

  Serial.println("Sensor v1.0a");
  airspeed_setup();
  Serial.println(" Booted...");
}

/**
 * @brief Main loop.  
 *
 * Performs:
 *   - USB serial command handling  
 */
void loop() {
  airspeed_loop();
}