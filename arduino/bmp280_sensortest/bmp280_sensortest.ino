/***************************************************************************
 ***************************************************************************/

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_BMP280.h>
#include <FastLED.h>
#include <EEPROM.h>
#include "esp_task_wdt.h"

#define EEPROM_SIZE 16
#define EEPROM_MAGIC_ADDR 0
#define EEPROM_QNH_ADDR   1
#define EEPROM_MAGIC      0x42

#define DEFAULT_QNH 1013.25f
#define SEA_LEVEL_PRESSURE 1013.25f  // hPa

#define SDA_PIN 7
#define SCL_PIN 6
#define BMP280_ADDRESS_ALT_LOCAL (0x76)

#define NUM_LEDS 1
#define DATA_PIN 48

#define WDT_TIMEOUT 5 // seconds

CRGB leds[NUM_LEDS];

float qnh = DEFAULT_QNH;
float altitude_offset = 0.0f;

Adafruit_BMP280 bmp;  // use I2C interface
Adafruit_Sensor *bmp_pressure = bmp.getPressureSensor();
Adafruit_Sensor *bmp_temp = bmp.getTemperatureSensor();

void setup() {
  Serial.begin(115200);
//  while (!Serial) delay(100);  // wait for native usb
  Serial.println(F("BMP280 Sensor event test..."));
  loadQNH();

  // Initialize Task Watchdog
  esp_task_wdt_config_t wdt_config = {
    .timeout_ms = WDT_TIMEOUT * 1000,
    .idle_core_mask = (1 << portNUM_PROCESSORS) - 1, // watch both cores
    .trigger_panic = true                            // reset on timeout
  };
  esp_task_wdt_init(&wdt_config);
  // Add loop() task to watchdog
  esp_task_wdt_add(NULL);


  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);

  // THIS IS THE IMPORTANT LINE
  Wire.begin(SDA_PIN, SCL_PIN);

  // Default settings from datasheet.
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,      // Operating Mode.
                  Adafruit_BMP280::SAMPLING_X2,      // Temp. oversampling
                  Adafruit_BMP280::SAMPLING_X16,     // Pressure oversampling
                  Adafruit_BMP280::FILTER_X16,       // Filtering.
                  Adafruit_BMP280::STANDBY_MS_500);  // Standby time.

  bmp_temp->printSensorDetails();

  unsigned status;
  status = bmp.begin(BMP280_ADDRESS_ALT, BMP280_CHIPID);
  //status = bmp.begin();
  if (!status) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring or "
                     "try a different address!"));
    Serial.print("SensorID was: 0x");
    Serial.println(bmp.sensorID(), 16);
    Serial.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
    Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
    Serial.print("        ID of 0x60 represents a BME 280.\n");
    Serial.print("        ID of 0x61 represents a BME 680.\n");
    delay(1000);
    ESP.restart();
  }
}

void loadQNH() {
  EEPROM.begin(EEPROM_SIZE);

  if (EEPROM.read(EEPROM_MAGIC_ADDR) == EEPROM_MAGIC) {
    EEPROM.get(EEPROM_QNH_ADDR, qnh);
    Serial.print("Loaded QNH from EEPROM: ");
    Serial.println(qnh, 2);
  } else {
    qnh = DEFAULT_QNH;
    Serial.println("No stored QNH, using default 1013.25");
  }
}

void saveQNH() {
  EEPROM.write(EEPROM_MAGIC_ADDR, EEPROM_MAGIC);
  EEPROM.put(EEPROM_QNH_ADDR, qnh);
  EEPROM.commit();

  Serial.print("Saved QNH to EEPROM: ");
  Serial.println(qnh, 2);
}

void calibrateAltitude(float pressure, float temp) {
  altitude_offset = pressureToAltitudeNEW(pressure, temp);
}

float getRelativeAltitude(float pressure, float temp) {
  return pressureToAltitudeNEW(pressure, temp) - altitude_offset;
}

float pressureToAltitudeNEW(float pressure_hPa, float temperature_C) {
  //void temperature_C;
  return 44330.0f * (1.0f - pow(pressure_hPa / qnh, 0.1903f));
}
/*
float pressureToAltitude(float pressure_hPa, float temperature_C) {
  // Convert Celsius to Kelvin
  float temperature_K = temperature_C + 273.15f;

  // Hypsometric equation
  float altitude_m =
    (287.05f * temperature_K / 9.80665f) *
    log(SEA_LEVEL_PRESSURE / pressure_hPa);

  return altitude_m;
}
*/
void handleSerialCommands(float pressure, float temperature) {
  if (!Serial.available()) return;

  String cmd = Serial.readStringUntil('\n');
  cmd.trim();

  if (cmd.equalsIgnoreCase("CAL")) {
    calibrateAltitude(pressure, temperature);
  }
  else if (cmd.startsWith("QNH=")) {
    float newQnh = cmd.substring(4).toFloat();
    if (newQnh > 900 && newQnh < 1100) {
      qnh = newQnh;
      saveQNH();
    } else {
      Serial.println("Invalid QNH value");
    }
  }
  else if (cmd.equalsIgnoreCase("QNH?")) {
    Serial.print("Current QNH = ");
    Serial.println(qnh, 2);
  }
  else {
    Serial.println("Unknown command");
  }
}

void loop() {
  const float alpha = 0.075; // Smoothing factor (0.0 to 1.0)
  static float filteredPressValue = 0;
  static float filteredTempValue = 0;
  static bool lo = false;
  static int calibrate = 0; // After 10 sec do calibrate...

  sensors_event_t temp_event, pressure_event;
  bmp_temp->getEvent(&temp_event);
  bmp_pressure->getEvent(&pressure_event);

  // The core filtering equation
  filteredPressValue = alpha * pressure_event.pressure + (1 - alpha) * filteredPressValue; 
  // Use filteredValue for further processing
  filteredTempValue  = alpha * temp_event.temperature + (1 - alpha) * filteredTempValue; 

  if(calibrate == 200){
    calibrateAltitude(filteredPressValue, filteredTempValue);
  }
  if(++calibrate >= 201){
    calibrate = 201;
  }

// Handle serial input
  handleSerialCommands(filteredPressValue, filteredTempValue);

  float altitude = pressureToAltitudeNEW(filteredPressValue,filteredTempValue);
  float relative = getRelativeAltitude(filteredPressValue,filteredTempValue);
  /*
  Serial.print(F("Temperature = "));
  Serial.print(temp_event.temperature);
  Serial.println(" *C");

  Serial.print(F("Pressure = "));
  Serial.println(pressure_event.pressure);
  Serial.println(" hPa");
  */

  String s = String(filteredPressValue, 4) + "," + 
              String(filteredTempValue, 4) + "," + 
              String(relative, 4) + "," + 
              String(altitude, 4);
  
  if (Serial.availableForWrite() > s.length()) {
    Serial.println(s);
  }

  if (lo == true) {
    //  leds[0] = CRGB::Red; FastLED.show(); delay(500);
    leds[0] = CRGB::Green;
    FastLED.show();
  } else {
    //  leds[0] = CRGB::Blue; FastLED.show(); delay(500);
    leds[0] = 0;
    FastLED.show();
  }
  delay(100);
  lo = !lo;

  esp_task_wdt_reset();
}
