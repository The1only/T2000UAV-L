#include <Wire.h>

#define MS4525_ADDR 0x28
#define AIR_DENSITY 1.225f

// Set this to your sensor range:
// ±1 PSI -> 1.0f
// ±2 PSI -> 2.0f
// ±5 PSI -> 5.0f
#define SENSOR_FULL_SCALE_PSI 1.0f
#define sdaPin  4
#define clkPin 5
#define DP_DEADBAND_PA 2.5f   // typical: 6–15 Pa

float pressureOffsetPa = 0.0f;

static bool readMs4525(uint16_t &rawP, uint16_t &rawT, uint8_t &status)
{
  if (Wire.requestFrom(MS4525_ADDR, (uint8_t)4) != 4)
    return false;

  uint8_t b1 = Wire.read();
  uint8_t b2 = Wire.read();
  uint8_t b3 = Wire.read();
  uint8_t b4 = Wire.read();

  status = (b1 >> 6) & 0x03;
  rawP = ((uint16_t)(b1 & 0x3F) << 8) | b2;
  rawT = ((uint16_t)b3 << 3) | (b4 >> 5);
  return true;
}

static float rawPressureToPa(uint16_t rawP)
{
  // 14-bit output, 10% to 90% span
  const float P_MIN = 0.1f * 16384.0f;  // 1638.4
  const float P_MAX = 0.9f * 16384.0f;  // 14745.6
  const float P_SPAN = P_MAX - P_MIN;
  const float P_MID = P_MIN + (P_SPAN * 0.5f);

  // counts from mid-span (can be +/-)
  float dpCounts = (float)rawP - P_MID;

  // scale to pressure using full-scale PSI
  float dpPsi = dpCounts / (P_SPAN * 0.5f) * SENSOR_FULL_SCALE_PSI;
  return dpPsi * 6894.76f; // PSI -> Pa
}

static void calibrateZero()
{
  const int samples = 200;
  float sum = 0.0f;
  int good = 0;

  for (int i = 0; i < samples; i++) {
    uint16_t rawP, rawT;
    uint8_t status;

    if (!readMs4525(rawP, rawT, status)) { delay(5); continue; }
    if (status != 0) { delay(5); continue; }

    float dpPa = rawPressureToPa(rawP);
    // Fix polarity if needed
    dpPa = -dpPa;
    sum += dpPa;
    good++;
    delay(5);
  }

  pressureOffsetPa = (good > 0) ? ((sum / good)+7.5) : 0.0f;

  Serial.print("Zero calibration: samples=");
  Serial.print(good);
  Serial.print(" offsetPa=");
  Serial.println(pressureOffsetPa, 2);
}

void setup() {
  Serial.begin(115200);
  uint32_t start = millis();
  while (!Serial && millis() - start < 5000) {
    delay(10);
  }
  Serial.println("Booting...");
  Wire.begin(sdaPin, clkPin, 400000);

  
  Serial.println("MS4525DO starting...");
  Serial.println("Keep the pitot still for zero calibration...");
  delay(500);

  calibrateZero();
}

void loop() {
  uint16_t rawP, rawT;
  uint8_t status;

  if (!readMs4525(rawP, rawT, status)) {
    Serial.println("I2C read failed");
    delay(100);
    return;
  }

  if (status != 0) {
    Serial.print("Status=");
    Serial.println(status);
    delay(100);
    return;
  }

  float dpPa = rawPressureToPa(rawP);           // uncorrected
  // Fix polarity if needed
  dpPa = -dpPa;
  
  float correctedPa = dpPa - pressureOffsetPa;  // corrected

  // Clamp: airspeed only for positive differential pressure
  float airspeed = 0.0f;
  if (correctedPa > 0.0f) {
    if (correctedPa > DP_DEADBAND_PA) {
      airspeed = sqrtf((2.0f * correctedPa) / AIR_DENSITY);
    }
  }

  Serial.print("Raw P: "); Serial.print(rawP);
  Serial.print(" | Raw T: "); Serial.print(rawT);
  Serial.print(" | dP (Pa): "); Serial.print(dpPa, 2);
  Serial.print(" | offset (Pa): "); Serial.print(pressureOffsetPa, 2);
  Serial.print(" | corrected (Pa): "); Serial.print(correctedPa, 2);
  Serial.print(" | Airspeed (m/s): "); Serial.println(airspeed, 2);

  delay(100);
}