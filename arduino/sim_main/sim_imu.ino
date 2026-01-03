/**
 * @file imu_device.cpp
 * @brief ESP32 Telnet bridge + SSDP (UPnP) device discovery + optional IMU simulator.
 *
 * This firmware exposes the ESP32-S3 as:
 *  - A Telnet-based IMU/radar/transponder data source (WIT protocol packets)
 *  - A UPnP/SSDP-discoverable network sensor
 *  - A simulator (if SIMULATE enabled)
 *
 * It also provides:
 *  - USB serial command interface for setting Wi-Fi credentials
 *  - Persistent Wi-Fi storage using NVS (Preferences)
 *  - SSDP responder for discovery by Qt, Android, macOS, and iOS hosts
 *
 * Main components:
 *  - @ref handleSsdpReceive() : Responds to SSDP M-SEARCH discovery
 *  - @ref sendPacket()        : Sends binary IMU packets to Telnet clients
 *  - @ref handleSerialConfig(): Interactive CLI for WiFi setup
 *  - @ref connectWiFi()       : Multi-AP WiFi connection logic
 */

#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiUdp.h>
#include <math.h>
#include <Preferences.h>

// --- Put these helpers somewhere (top of file) ---
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// -----------------------------------------------------------------------------
// Timing / LED
// -----------------------------------------------------------------------------
#define LED 8
static const uint16_t PERIOD_MS_IMU = 20;  ///< Loop delay when simulating IMU (~50 Hz)
// -----------------------------------------------------------------------------
// IMU Packet Conversion Helpers
// -----------------------------------------------------------------------------

/**
 * @brief Clamp a 32-bit value into signed 16-bit range.
 * @param v Input value
 * @return int16_t Clamped output
 */
static int16_t clamp16(long v) {
  if (v > 32767) return 32767;
  if (v < -32768) return -32768;
  return (int16_t)v;
}

/**
 * @brief Convert angle in degrees to WIT-protocol Q15 format.
 * @param deg Angle in degrees (±180)
 * @return int16_t Scaled Q15 value
 */
// scale degrees -> protocol int16 (±180°)
static int16_t angle_to_q15(float deg) {
  return clamp16((long)(deg * 32768.0f / 180.0f));
}

/**
 * @brief Convert Celsius temperature into the WIT format raw value.
 * @param celsius Temperature
 * @return uint16_t Encoded temperature
 */
static uint16_t make_temp_raw(float celsius = 25.0f) {
  float raw = (celsius - 36.53f) * 340.0f;
  long r = (long)(raw);
  if (r < 0) r = 0;
  if (r > 65535) r = 65535;
  return (uint16_t)r;
}


// --- Simulator encoding helpers (MATCH PARSER) ---

static int16_t sim_accel_g(float g) {
  return clamp16((long)(g / 16.0f * 32768.0f));
}

static int16_t sim_gyro_dps(float dps) {
  return clamp16((long)(dps / 2000.0f * 32768.0f));
}

static int16_t sim_angle_deg(float deg) {
  return clamp16((long)(deg / 180.0f * 32768.0f));
}

static int16_t sim_temp_c(float c) {
  return (int16_t)(c * 100.0f);
}

static uint32_t sim_pressure_pa(float pa) {
  return (uint32_t)(pa * 100.0f);
}
// scale pressure hPa -> protocol int16 (hPa * 10)
static int16_t hpa_to_q10(float hpa) {
  return clamp16((long)(hpa * 10.0f));
}

/**
 * @brief Convert dps (degrees/sec) to Q15 (±2000 dps)
 */
static int16_t dps_to_q15(float dps) {
  return clamp16((long)(dps * 32768.0f / 2000.0f));
}

/**
 * @brief Convert G-force to Q15 (±16 g)
 */
static int16_t g_to_q15(float g) {
  return clamp16((long)(g * 32768.0f / 16.0f));
}

// Send a 32-bit value split across two consecutive registers
static void sendPacket32(uint8_t pid, uint32_t value) {
  int16_t low = (int16_t)(value & 0xFFFF);
  int16_t high = (int16_t)((value >> 16) & 0xFFFF);

  // WIT convention: X=low, Y=high, Z=unused
  sendPacket(pid, low, high, 0, make_temp_raw(25.0f));
}

// 4x16-bit payload (the last 16-bit uses the temp slot)
static void sendPacket4(uint8_t pid, int16_t a, int16_t b, int16_t c, int16_t d) {
  sendPacket(pid, a, b, c, (uint16_t)d);
}

static inline uint16_t lo16(uint32_t v) {
  return (uint16_t)(v & 0xFFFF);
}
static inline uint16_t hi16(uint32_t v) {
  return (uint16_t)((v >> 16) & 0xFFFF);
}

/**
 * Encode degrees -> NMEA ddmm.mmmm scaled integer.
 * Example: 59.9123 deg -> 5954.7380 (ddmm.mmmm)
 * We scale by 10000 to keep 4 decimals in minutes.
 *
 * IMPORTANT: This must match what your nmea_ddmm_to_deg() expects.
 * If your real sensor uses a different scaling, adjust SCALE.
 */
static int32_t deg_to_ddmm_scaled(double deg) {
  const double SCALE = 10000.0;  // ddmm.mmmm * 10000
  double a = fabs(deg);
  int d = (int)a;
  double minutes = (a - d) * 60.0;
  double ddmm = (double)(d * 100) + minutes;  // ddmm.mmmm (as double)
  int32_t raw = (int32_t)llround(ddmm * SCALE);
  // If you need signed hemisphere support, you can keep sign:
  if (deg < 0) raw = -raw;
  return raw;
}
/**
 * @brief Send an 11-byte WIT IMU packet to all Telnet clients.
 *
 * Packet layout:
 *   Byte 0:  0x55 (header)
 *   Byte 1:  PID (e.g. 0x53 = angles)
 *   Byte 2-3: X low/high
 *   Byte 4-5: Y low/high
 *   Byte 6-7: Z low/high
 *   Byte 8-9: temperature raw
 *   Byte 10 : checksum
 *
 * @param pid Packet ID
 * @param x X-axis  value (Q15)
 * @param y Y-axis  value (Q15)
 * @param z Z-axis  value (Q15)
 * @param tRaw Raw temperature value
 */
static void sendPacket(uint8_t pid, int16_t x, int16_t y, int16_t z, uint16_t tRaw) {
  uint8_t pkt[11];

  pkt[0] = 0x55;
  pkt[1] = pid;

  pkt[2] = x & 0xFF;
  pkt[3] = (x >> 8) & 0xFF;

  pkt[4] = y & 0xFF;
  pkt[5] = (y >> 8) & 0xFF;

  pkt[6] = z & 0xFF;
  pkt[7] = (z >> 8) & 0xFF;

  pkt[8] = tRaw & 0xFF;
  pkt[9] = (tRaw >> 8) & 0xFF;

  uint16_t sum = 0;
  for (int i = 0; i < 10; ++i) sum += pkt[i];
  pkt[10] = sum & 0xFF;

  for (int i = 0; i < MAX_SRV_CLIENTS; i++) {
    if (serverClients[i] && serverClients[i].connected()) {
      serverClients[i].write(pkt, sizeof(pkt));
    }
  }
}


/**
 * @brief Forwards Telnet input into Serial1 (UART bridge mode only).
 * @param activeFlag Set to true if data received
 */
static void imu_handle_data(char ch) {
  static int state = 0;

  // Wee need to spy on the communication to ba able to adapt to the changes...
  uint8_t inChar = ch;
  switch (state) {
    case 0:
      if (inChar == 0xFF) state = 1;
      break;

    case 1:
      if (inChar == 0xAA) state = 2;
      else
        state = 0;
      break;

    case 2:
      switch (inChar) {
        case 0x63:
        //  Serial1.begin(15200);
          state = 0;
          break;
        case 0x64:
        //  Serial1.begin(9600);
          state = 0;
          break;
        case 0x27:
          state = 3;
          break;
        default:
          state = 0;
          break;
      }
      break;

    case 3:
      if (inChar == 0x34)
        active = true;
      else
        active = false;
      state = 4;
      break;

    case 4:
    default:
      state = 0;
      break;
  }
}

// -----------------------------------------------------------------------------
// Setup / Loop
// -----------------------------------------------------------------------------

/**
 * @brief Arduino setup. Initializes Wi-Fi, SSDP, Telnet, USART (optional).
 */
void imu_setup() {
  Serial1.begin(9600, SERIAL_8N1, 5, 4); // Baud, format, RX pin, TX pin (check your board!)
}

/**
 * @brief Main loop.  
 *
 * Performs:
 *   - Wi-Fi reconnection watchdog  
 *   - USB serial command handling  
 *   - SSDP discovery responder  
 *   - Telnet server operations  
 *   - Optional IMU simulation (if SIMULATE defined)
 */
void imu_loop() {
  static int sleep = 0;
  int tx = millis();

  static int tdo = 0;
  if (tx - tdo > 50) {
    tdo = tx;

    static float offset=0.0;

    // ---- Time base ----
    const float tf = millis() * 0.001f;  // seconds

    // ---- Angle signals (deg) ----
    // (You asked to half roll angular rate -> use 0.1 Hz if it was 0.2 Hz)
    const float f_roll = 0.02f;  // Hz
    const float f_pitch = 0.1f;  // Hz
    const float f_yaw = 0.02f;   // Hz
    const float W_roll = 2.0f * (float)M_PI * f_roll;
    const float W_pitch = 2.0f * (float)M_PI * f_pitch;
    const float W_yaw = 0.5f * (float)M_PI * f_pitch;

    const float roll = 50.0f * sinf(W_roll * tf);    // deg
    const float pitch = 20.0f * sinf(W_pitch * tf);  // deg
    const float yaw = 100.0f * sinf(W_yaw * tf);   // deg
                                                      //    const float yaw   = fmodf(tf * 20.0f, 360.0f) - 180.0f;         // deg (20 deg/s)

    // ---- Gyro rates (deg/s) = time-derivative of angles ----
    const float roll_rate_dps = 30.0f * W_roll * cosf(W_roll * tf);
    const float pitch_rate_dps = 10.0f * W_pitch * cosf(W_pitch * tf);
    const float yaw_rate_dps = 30.0f * W_yaw * cosf(W_yaw * tf);
    //    const float yaw_rate_dps   = 20.0f;

    // Convert to raw (±2000 dps -> 32768)
    // Match your parser sign conventions:
    //   AsZ from GX+0 (no sign flip)
    //   AsY from GX+1 BUT parser flips sign => store -pitch_rate here
    //   AsX from GX+2 (no sign flip)
    const int16_t gx_raw = clamp16((long)(roll_rate_dps * 32768.0f / 2000.0f));    // maps to AsZ
    const int16_t gy_raw = clamp16((long)(-pitch_rate_dps * 32768.0f / 2000.0f));  // maps to AsY after parser's -1
    const int16_t gz_raw = clamp16((long)(yaw_rate_dps * 32768.0f / 2000.0f));     // maps to AsX

    // ---- Accelerometer (gravity projected into body frame) ----
    // Only gravity (no linear accel). g matches your parser.
    const float g = 9.825f;

    const float roll_r = roll * (float)DEG_TO_RAD;
    const float pitch_r = pitch * (float)DEG_TO_RAD;

    // gravity in body axes (m/s^2)
    const float ax_ms2 = -sinf(pitch_r) * g;
    const float ay_ms2 = -sinf(roll_r) * cosf(pitch_r) * g;
    const float az_ms2 = cosf(roll_r) * cosf(pitch_r) * g;

    // Convert m/s^2 -> raw for ±16g:
    // Parser does: Acc? = ((raw/32768)*16)*g   (then AccX has extra -1)
    // So: raw = acc/(g*16) * 32768
    const int16_t ax_raw = clamp16((long)(ax_ms2 / (g * 16.0f) * 32768.0f));
    const int16_t ay_raw = clamp16((long)(ay_ms2 / (g * 16.0f) * 32768.0f));
    const int16_t az_raw = clamp16((long)(az_ms2 / (g * 16.0f) * 32768.0f));

    // ---- Temperature ----
    const uint16_t t_raw = make_temp_raw(25.0f);

    // ---------- Pressure (Pa) ----------
    float pressure_pa =
      90662.5f +  // midpoint
      10662.5f * sinf(2.0f * M_PI * (1.0f / 360.0f) * tf);
    // Range: 80000 ↔ 101325 Pa

    uint32_t p_raw = sim_pressure_pa(pressure_pa) / 100;

    // ---------- GPS simulation ----------
    const double baseLat = 59.9127;  // Oslo-ish (change)
    const double baseLon = 10.7461;

    const double gpsRadiusDeg = 0.002;                   // ~200 m-ish (roughly)
    const double gpsOmega = 2.0 * M_PI * (1.0 / 120.0);  // one loop every 120 s

    double lat = baseLat + gpsRadiusDeg * sin(gpsOmega * tf);
    double lon = baseLon + gpsRadiusDeg * cos(gpsOmega * tf);

    // Encode as NMEA ddmm.mmmm * 10000 (32-bit)
    int32_t lat_raw32 = deg_to_ddmm_scaled(lat);
    int32_t lon_raw32 = deg_to_ddmm_scaled(lon);

    // Altitude in meters -> D0Status is /10.0 in parser, so store *10
    float alt_m = 120.0f + 20.0f * sinf(2.0f * (float)M_PI * (1.0f / 180.0f) * tf);  // 100..140m
    int16_t alt_raw = (int16_t)clamp16((long)(alt_m * 10.0f));

    // GPS yaw in degrees -> parser does /100.0, so store *100
    float gps_yaw_deg = yaw;  // you already simulate yaw
    int16_t gpsYaw_raw = (int16_t)clamp16((long)(gps_yaw_deg * 100.0f));

    // GPS speed (your parser does join32()/100.0)
    // So store speed*100 as 32-bit. Choose units you want (e.g. knots, m/s) but be consistent.
    // Example: 12.3 (units) -> raw=1230
    float ttt = tf;
    float delta = 0.03f * (float)M_PI * ttt;
    float speed_units = offset + 20000.0f * sinf(delta);
    if(speed_units < 0) speed_units*=-1;
    uint32_t speed_raw32 = (uint32_t)lroundf(speed_units);
//      Serial.print("raw100=");
//    Serial.println(speed_units);

    // ---- Send WIT packets ----
    // 0x51 Acc, 0x52 Gyro, 0x53 Angles
    sendPacket(0x51, ax_raw, ay_raw, az_raw, t_raw);
    delay(PERIOD_MS_IMU);

    sendPacket(0x52, gx_raw, gy_raw, gz_raw, t_raw);
    delay(PERIOD_MS_IMU);

    // AngleZ in your parser has an extra -1, so store -yaw to match
    sendPacket(0x53,
                angle_to_q15(roll),
                angle_to_q15(pitch),
                angle_to_q15(-yaw),
                t_raw);
    delay(PERIOD_MS_IMU);

    // Send pressure as 32-bit value split into PressureL / PressureH
    sendPacket32(0x56, p_raw);  // helper that writes L/H regs
    delay(PERIOD_MS_IMU);

    // ----- Send GPS packets -----
    // 0x57: Lon32 + Lat32  (x=LonL,y=LonH,z=LatL,t=LatH)
    sendPacket4(0x57,
                (int16_t)lo16((uint32_t)lon_raw32),
                (int16_t)hi16((uint32_t)lon_raw32),
                (int16_t)lo16((uint32_t)lat_raw32),
                (int16_t)hi16((uint32_t)lat_raw32));
    delay(PERIOD_MS_IMU);

    // 0x58: Speed32 + GPSYaw16 + Alt16  (x=GPSVL,y=GPSVH,z=GPSYAW,t=D0Status)
    sendPacket4(0x58,
                (int16_t)lo16(speed_raw32),
                (int16_t)hi16(speed_raw32),
                gpsYaw_raw,
                alt_raw);
    delay(PERIOD_MS_IMU);
  }
}