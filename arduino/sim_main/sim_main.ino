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

#define SIMULATE

#define SENSOR_RADAR 1
#define SENSOR_IMU 2
#define SENSOR_TRANSPONDER 3

#define SENSOR_V SENSOR_IMU  // set this manually or with build flags

#if SENSOR_V == SENSOR_RADAR
#define SENSOR "RADAR"  //"IMU"              ///< Sensor identifier reported in SSDP USN
#endif

#if SENSOR_V == SENSOR_IMU
#define SENSOR "IMU"  //"IMU"              ///< Sensor identifier reported in SSDP USN
#endif

#if SENSOR_V == SENSOR_TRANSPONDER
#define SENSOR "TRANSPONDER"  //"IMU"              ///< Sensor identifier reported in SSDP USN
#endif

#define SENSOR_TYPE "Airplane-device"  ///< SSDP search target (ST)

#define MAX_SRV_CLIENTS 4  ///< Maximum simultaneous Telnet clients
#define USE_SERIAL true    ///< Use UART bridge instead of IMU simulation

// --- Put these helpers somewhere (top of file) ---
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// -----------------------------------------------------------------------------
// Timing / LED
// -----------------------------------------------------------------------------
#define LED 8
static const uint16_t PERIOD_MS = 20;  ///< Loop delay when simulating IMU (~50 Hz)

// -----------------------------------------------------------------------------
// Wi-Fi Credential Storage (NVS)
// -----------------------------------------------------------------------------

Preferences prefs;  ///< ESP32 NVS storage instance

const char *PREF_NAMESPACE = "wifi";  ///< Namespace in NVS for Wi-Fi settings
const char *KEY_SSID = "ssid";        ///< Key for stored SSID
const char *KEY_PASS = "pass";        ///< Key for stored password

bool hasStoredWifi = false;  ///< True if credentials exist in NVS
String storedSsid;           ///< Stored SSID value
String storedPass;           ///< Stored password value

String serialLine;  ///< Input buffer for USB-serial commands

// -----------------------------------------------------------------------------
// WiFi / Telnet Server
// -----------------------------------------------------------------------------

WiFiMulti wifiMulti;  ///< Manages multiple fallback WiFi APs

// Hardcoded fallback Wi-Fi APs (optional)
const char *ssid2 = "Altibox177449";
const char *pass2 = "HheX9Xac";

const char *ssid1 = "Hvattum";
const char *pass1 = "Jordvarme@2023@";

const char *ssid3 = "Aeros2";
const char *pass3 = "Terjenilsen1";

const char *ssid0 = "9Tek_printer";
const char *pass0 = "didiinne";

WiFiServer server(23);                      ///< Telnet server
WiFiClient serverClients[MAX_SRV_CLIENTS];  ///< Connected clients array

// -----------------------------------------------------------------------------
// SSDP / UPnP Discovery
// -----------------------------------------------------------------------------

/**
 * @brief SSDP multicast address as defined by UPnP (239.255.255.250:1900)
 */
const IPAddress SSDP_MCAST_ADDR("255.255.255.255");

/**
 * @brief Port for UPnP discovery (1900)
 */
const uint16_t SSDP_PORT = 4210;

/**
 * @brief SSDP Search Target (ST) string (e.g. "imu-device", "radar-device")
 */
const char *SSDP_ST = SENSOR_TYPE;

WiFiUDP ssdpUdp;  ///< UDP socket for SSDP traffic

bool active = true;  //false;
// -----------------------------------------------------------------------------
// IMU Packet Conversion Helpers
// -----------------------------------------------------------------------------

/**
 * @brief Clamp a 32-bit value into signed 16-bit range.
 * @param v Input value
 * @return int16_t Clamped output
 */
int16_t clamp16(long v) {
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
int16_t angle_to_q15(float deg) {
  return clamp16((long)(deg * 32768.0f / 180.0f));
}

/**
 * @brief Convert Celsius temperature into the WIT format raw value.
 * @param celsius Temperature
 * @return uint16_t Encoded temperature
 */
uint16_t make_temp_raw(float celsius = 25.0f) {
  float raw = (celsius - 36.53f) * 340.0f;
  long r = (long)(raw);
  if (r < 0) r = 0;
  if (r > 65535) r = 65535;
  return (uint16_t)r;
}


// --- Simulator encoding helpers (MATCH PARSER) ---

int16_t sim_accel_g(float g) {
  return clamp16((long)(g / 16.0f * 32768.0f));
}

int16_t sim_gyro_dps(float dps) {
  return clamp16((long)(dps / 2000.0f * 32768.0f));
}

int16_t sim_angle_deg(float deg) {
  return clamp16((long)(deg / 180.0f * 32768.0f));
}

int16_t sim_temp_c(float c) {
  return (int16_t)(c * 100.0f);
}

uint32_t sim_pressure_pa(float pa) {
  return (uint32_t)(pa * 100.0f);
}
// scale pressure hPa -> protocol int16 (hPa * 10)
int16_t hpa_to_q10(float hpa) {
  return clamp16((long)(hpa * 10.0f));
}

/**
 * @brief Convert dps (degrees/sec) to Q15 (±2000 dps)
 */
int16_t dps_to_q15(float dps) {
  return clamp16((long)(dps * 32768.0f / 2000.0f));
}

/**
 * @brief Convert G-force to Q15 (±16 g)
 */
int16_t g_to_q15(float g) {
  return clamp16((long)(g * 32768.0f / 16.0f));
}

// Send a 32-bit value split across two consecutive registers
void sendPacket32(uint8_t pid, uint32_t value) {
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
void sendPacket(uint8_t pid, int16_t x, int16_t y, int16_t z, uint16_t tRaw) {
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

// -----------------------------------------------------------------------------
// Wi-Fi Credential Management (NVS)
// -----------------------------------------------------------------------------
void WiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info) {
  Serial.print("WiFiEvent:");
  if (event == ARDUINO_EVENT_WIFI_STA_DISCONNECTED) {
    Serial.print("DISCONNECTED reason=");
    Serial.println(info.wifi_sta_disconnected.reason);
  }
}

/**
 * @brief Loads Wi-Fi SSID/password from ESP32 NVS storage.
 *
 * Populates @ref storedSsid, @ref storedPass and sets @ref hasStoredWifi accordingly.
 */
void loadStoredCredentials() {
  prefs.begin(PREF_NAMESPACE, true);  // read-only
  storedSsid = prefs.getString(KEY_SSID, "");
  storedPass = prefs.getString(KEY_PASS, "");
  prefs.end();

  if (storedSsid.length() > 0) {
    hasStoredWifi = true;
    Serial.printf("Found stored WiFi credentials: %s\n", storedSsid.c_str());
  } else {
    hasStoredWifi = false;
    Serial.println("No stored WiFi credentials.");
  }
}

/**
 * @brief Saves new Wi-Fi credentials into NVS.
 * @param ssid SSID string
 * @param pass Password string
 */
void saveCredentials(const String &ssid, const String &pass) {
  prefs.begin(PREF_NAMESPACE, false);
  prefs.putString(KEY_SSID, ssid);
  prefs.putString(KEY_PASS, pass);
  prefs.end();

  Serial.println("WiFi credentials saved to NVS.");
}

/**
 * @brief Clears stored Wi-Fi credentials from NVS.
 */
void clearCredentials() {
  prefs.begin(PREF_NAMESPACE, false);
  prefs.clear();
  prefs.end();
  hasStoredWifi = false;
  storedSsid.clear();
  storedPass.clear();
  Serial.println("Stored WiFi credentials erased.");
}

/**
 * @brief Adds stored Wi-Fi credentials (if any) and hardcoded fallback APs.
 */
void addAccessPoints() {
  if (hasStoredWifi)
    wifiMulti.addAP(storedSsid.c_str(), storedPass.c_str());

  wifiMulti.addAP(ssid0, pass0);
  wifiMulti.addAP(ssid1, pass1);
  wifiMulti.addAP(ssid2, pass2);
  wifiMulti.addAP(ssid3, pass3);
}

/**
 * @brief Attempts to connect to Wi-Fi using @ref WiFiMulti.
 * @return true on success, false if connection fails after multiple retries
 */
bool connectWiFi() {
  Serial.println("Connecting WiFi...");
  for (int loops = 20; loops > 0; --loops) {
    if (wifiMulti.run() == WL_CONNECTED) {
      Serial.print("WiFi connected. IP: ");
      Serial.println(WiFi.localIP());
      Serial.print("Connected SSID: ");
      Serial.println(WiFi.SSID());
      Serial.print("RSSI: ");
      Serial.println(WiFi.RSSI());
      return true;
    }
    delay(400);
  }
  return false;
}

// -----------------------------------------------------------------------------
// SSDP Discovery Handling
// -----------------------------------------------------------------------------

/**
 * @brief Parses incoming SSDP (UPnP) M-SEARCH requests and sends a proper SSDP
 *        response if the Search Target (ST) matches our @ref SENSOR_TYPE.
 *
 * SSDP flows:
 *   1. Host sends M-SEARCH to 239.255.255.250:1900
 *   2. Device responds unicast to the sender with a 200 OK SSDP packet.
 *
 * @return true if a valid request was processed, false otherwise.
 */
bool handleSsdpReceive() {
  int packetSize = ssdpUdp.parsePacket();
  if (packetSize <= 0) {
    delay(10);
    return false;
  }

  char buf[1024];
  int len = ssdpUdp.read(buf, sizeof(buf) - 1);
  if (len <= 0) return false;
  buf[len] = '\0';

  String req(buf);

  if (!req.startsWith("M-SEARCH") && req.indexOf("M-SEARCH") < 0)
    return false;

  bool stMatch = false;
  if (req.indexOf(String("ST: ") + SSDP_ST) >= 0) stMatch = true;
  if (req.indexOf("ST: ssdp:all") >= 0) stMatch = true;

  if (!stMatch) return false;
  send_ssdp_blind();
  return true;
}

void send_ssdp_blind() {
  WiFiUDP udp;
  IPAddress bcast(SSDP_MCAST_ADDR);  // or your subnet’s broadcast
  udp.beginPacket(bcast, SSDP_PORT);

  IPAddress ip = WiFi.localIP();
  String location = "http://" + ip.toString() + ":80/" + String(SENSOR_TYPE) + ".xml";

  String resp =
    "NOTIFY * HTTP/1.1 200 OK\r\n"
    "CACHE-CONTROL: max-age=60\r\n"
    "EXT:\r\n"
    "ST: "
    + String(SSDP_ST) + "\r\n" + "USN: " + String(SENSOR) + "\r\n" + "LOCATION: " + location + "\r\n"
                                                                                               "SERVER: ESP32/1.0 UPnP/1.1 "
    + String(SENSOR) + "/1.0\r\n"
                       "\r\n";

  // Serial.println(resp);
  udp.print(resp);  // or some JSON payload
  udp.endPacket();
}

// -----------------------------------------------------------------------------
// Telnet Handling
// -----------------------------------------------------------------------------

/**
 * @brief Accepts new Telnet clients up to @ref MAX_SRV_CLIENTS.
 *
 * Old/disconnected sockets are automatically reused.
 */
void acceptNewClients() {
  if (!server.hasClient()) return;

  for (int i = 0; i < MAX_SRV_CLIENTS; i++) {
    if (!serverClients[i] || !serverClients[i].connected()) {
      if (serverClients[i]) serverClients[i].stop();
      serverClients[i] = server.available();
      Serial.print("Client server: ");
      Serial.println(i);
      return;
    }
  }

  WiFiClient tmp = server.available();
  if (tmp) tmp.stop();  // reject extra client

  Serial.print("error server:");
}

/**
 * @brief Forwards Telnet input into Serial1 (UART bridge mode only).
 * @param activeFlag Set to true if data received
 */
void handleTelnetToSerial(bool &activeFlag) {
  static int state = 0;

  for (int i = 0; i < MAX_SRV_CLIENTS; i++) {
    if (serverClients[i] && serverClients[i].connected()) {
      while (serverClients[i].available()) {
        char ch = serverClients[i].read();
        activeFlag = true;
        //Serial.print((char)ch);


#if SENSOR_V == SENSOR_TRANSPONDER
        handle_data(ch);
#else
      //  Serial1.write(ch);

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
#endif
      }
    }
  }
}

/**
 * @brief Forwards UART Serial1 data to all connected Telnet clients.
 */
void handleSerialToTelnet() {
  if (Serial1.available()) {
    size_t len = Serial1.available();
    uint8_t sbuf[256];
    if (len > sizeof(sbuf)) len = sizeof(sbuf);

    Serial1.readBytes(sbuf, len);

    for (int i = 0; i < MAX_SRV_CLIENTS; i++)
      if (serverClients[i] && serverClients[i].connected())
        serverClients[i].write(sbuf, len);
  }
}

// -----------------------------------------------------------------------------
// USB Serial CLI (WiFi config commands)
// -----------------------------------------------------------------------------

/**
 * @brief Prints available USB-serial commands to the user.
 */
void printHelp() {
  Serial.println("Commands:");
  Serial.println("  help                - show help");
  Serial.println("  showwifi            - show stored Wi-Fi credentials");
  Serial.println("  wifi <ssid> <pass>  - store credentials and reboot");
  Serial.println("  clearwifi           - erase stored Wi-Fi");
  Serial.println("  boot                - reboot");
}

/**
 * @brief Parses and executes a CLI command entered over USB serial.
 * @param line Full input command
 */
void processSerialCommand(const String &line) {
  String cmd = line;
  cmd.trim();
  if (cmd.length() == 0) return;

  if (cmd.equalsIgnoreCase("help")) {
    printHelp();
    return;
  }

  if (cmd.equalsIgnoreCase("boot")) {
    delay(100);
    ESP.restart();
    return;
  }

  if (cmd.equalsIgnoreCase("showwifi")) {
    if (hasStoredWifi) {
      Serial.printf("SSID: %s\n", storedSsid.c_str());
      Serial.printf("PASS: %s\n", storedPass.c_str());
    } else {
      Serial.println("No stored Wi-Fi.");
    }
    return;
  }

  if (cmd.equalsIgnoreCase("clearwifi")) {
    clearCredentials();
    return;
  }

  if (cmd.startsWith("wifi ")) {
    int s1 = cmd.indexOf(' ');
    int s2 = cmd.indexOf(' ', s1 + 1);
    if (s2 < 0) {
      Serial.println("Usage: wifi <ssid> <pass>");
      return;
    }
    String ssid = cmd.substring(s1 + 1, s2);
    String pass = cmd.substring(s2 + 1);
    ssid.replace('_', ' ');
    pass.replace('_', ' ');

    saveCredentials(ssid, pass);
    Serial.println("Rebooting...");
    delay(500);
    ESP.restart();
    return;
  }

  printHelp();
  Serial.println("Unknown command.");
}

/**
 * @brief Collects characters from USB serial and builds command lines.
 *
 * Called every loop iteration.
 */
void handleSerialConfig() {
  while (Serial.available() > 0) {
    char c = Serial.read();
    if (c == '\n') {
      processSerialCommand(serialLine);
      serialLine = "";
    } else if (c != '\r') {
      serialLine += c;
    }
  }
}

// -----------------------------------------------------------------------------
// Setup / Loop
// -----------------------------------------------------------------------------

/**
 * @brief Arduino setup. Initializes Wi-Fi, SSDP, Telnet, USART (optional).
 */
void setup() {

  // Set up LED...
  pinMode(LED, OUTPUT);

  Serial.begin(9600);
  loadStoredCredentials();
  addAccessPoints();

  Serial.println("Hitt a key to stop booting...");
  for (int i = 0; i < 5000; i++) {
    if (Serial.available()) {
      while (1) handleSerialConfig();
    }
    delay(1);
  }
  Serial.println("booting...");

  WiFi.onEvent(WiFiEvent);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true);    // clear old connection
  delay(100);

  // --- optional one-time scan to verify SSID exists ---
  Serial.println("Scanning for networks...");
  int n = WiFi.scanNetworks();
  if (n <= 0) {
    Serial.println("No networks found");
  } else {
    Serial.printf("%d networks found:\n", n);
    for (int i = 0; i < n; ++i) {
      Serial.printf("  %2d: %s (%d dBm)%s\n",
                    i + 1,
                    WiFi.SSID(i).c_str(),
                    WiFi.RSSI(i),
                    (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? "" : " *");
    }
  }
  Serial.println("----");

  if (!connectWiFi()) {
    Serial.println("WiFi failed, rebooting...");
    ESP.restart();
  }

  ssdpUdp.beginMulticast(SSDP_MCAST_ADDR, SSDP_PORT);
  server.begin();
  server.setNoDelay(true);

#if SENSOR_V == SENSOR_RADAR
//  Serial1.begin(115200);
  setupRADAR();
#elif SENSOR_V == SENSOR_TRANSPONDER
//  Serial1.begin(9600);
  setupTRANSPONDER();
#else
 // Serial1.begin(9600);
#endif

  Serial.print("v1.0 ");
  Serial.print(SENSOR);
  Serial.println(" Booted...");
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
void loop() {
  static int sleep = 0;
  bool hasclient = false;
  int tx = millis();

  if (wifiMulti.run() != WL_CONNECTED) {
    Serial.println("WiFi lost!");
    delay(500);
    return;
  }

  // If there are client connected, then keep sending SSDP messages.
  // The reason for this is tha tthe iPhone and iPad does not send SSDP requests, but can receive...
  for (int i = 0; i < MAX_SRV_CLIENTS; i++) {
    if (serverClients[i] && serverClients[i].connected()) {
      hasclient = true;
    }
  }

  if (!hasclient) {
    static int td = 0;
    static bool xx = false;
    if (tx - td > 2500) {
      td = tx;
      send_ssdp_blind();
      xx = !xx;
      if(xx) {
        digitalWrite(LED, LOW);
      }
      else{
        digitalWrite(LED, HIGH);
      }
    }
  }

  handleSsdpReceive();
  acceptNewClients();

#if SENSOR_V == SENSOR_RADAR
  loopRADAR();
#endif

#if SENSOR_V == SENSOR_TRANSPONDER
  loopTRANSPONDER();
  handleTelnetToSerial(active);
  handleSerialToTelnet();
  delay(100);
#endif @

#if SENSOR_V == SENSOR_IMU
#ifndef SIMULATE
  handleTelnetToSerial(active);
  handleSerialToTelnet();
#else

  if (hasclient) {
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
      delay(PERIOD_MS);

      sendPacket(0x52, gx_raw, gy_raw, gz_raw, t_raw);
      delay(PERIOD_MS);

      // AngleZ in your parser has an extra -1, so store -yaw to match
      sendPacket(0x53,
                 angle_to_q15(roll),
                 angle_to_q15(pitch),
                 angle_to_q15(-yaw),
                 t_raw);
      delay(PERIOD_MS);

      // Send pressure as 32-bit value split into PressureL / PressureH
      sendPacket32(0x56, p_raw);  // helper that writes L/H regs
      delay(PERIOD_MS);

      // ----- Send GPS packets -----
      // 0x57: Lon32 + Lat32  (x=LonL,y=LonH,z=LatL,t=LatH)
      sendPacket4(0x57,
                  (int16_t)lo16((uint32_t)lon_raw32),
                  (int16_t)hi16((uint32_t)lon_raw32),
                  (int16_t)lo16((uint32_t)lat_raw32),
                  (int16_t)hi16((uint32_t)lat_raw32));
      delay(PERIOD_MS);

      // 0x58: Speed32 + GPSYaw16 + Alt16  (x=GPSVL,y=GPSVH,z=GPSYAW,t=D0Status)
      sendPacket4(0x58,
                  (int16_t)lo16(speed_raw32),
                  (int16_t)hi16(speed_raw32),
                  gpsYaw_raw,
                  alt_raw);
      delay(PERIOD_MS);
    }
  }

#endif
#endif
}