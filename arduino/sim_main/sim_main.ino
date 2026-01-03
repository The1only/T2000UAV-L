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
#include <Adafruit_BMP280.h>
#include "esp_task_wdt.h"

#define USE_FastLED
//#undef USE_FastLED

#ifdef USE_FastLED
#include <FastLED.h>
#endif

#define SENSOR_RADAR 1
#define SENSOR_IMU 2
#define SENSOR_TRANSPONDER 3
#define SENSOR_ALTIMETER 4
#define SENSOR_AIRSPEED 5

#define SENSOR_V SENSOR_AIRSPEED  // set this manually or with build flags

#if SENSOR_V == SENSOR_RADAR
#define SENSOR "RADAR"  //              ///< Sensor identifier reported in SSDP USN
#elif SENSOR_V == SENSOR_IMU
#define SENSOR "IMU"  //              ///< Sensor identifier reported in SSDP USN
#elif SENSOR_V == SENSOR_TRANSPONDER
#define SENSOR "TRANSPONDER"  //              ///< Sensor identifier reported in SSDP USN
#elif SENSOR_V == SENSOR_ALTIMETER
#define SENSOR "ALTIMETER"  //              ///< Sensor identifier reported in SSDP USN
#elif SENSOR_V == SENSOR_AIRSPEED
#define SENSOR "AIRSPEED"  //              ///< Sensor identifier reported in SSDP USN
#endif

#define SENSOR_TYPE "Airplane-device"  ///< SSDP search target (ST)

#define MAX_SRV_CLIENTS 4  ///< Maximum simultaneous Telnet clients
#define USE_SERIAL true    ///< Use UART bridge instead of IMU simulation

// --- Put these helpers somewhere (top of file) ---
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define NUM_LEDS 1
#define DATA_PIN 48
#define WDT_TIMEOUT 5  // seconds

#ifdef USE_FastLED
CRGB leds[NUM_LEDS];
#endif

bool SIMULATE = false;

// -----------------------------------------------------------------------------
// Timing / LED
// -----------------------------------------------------------------------------
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

#if SENSOR_V == SENSOR_TRANSPONDER
        handle_data(ch);

#elif SENSOR_V == SENSOR_IMU
        if(SIMULATE == true){
          imu_handle_data(ch);
        }
        else{
          Serial1.print((char)ch);
        }
#elif SENSOR_V == SENSOR_RADAR
// ..
#elif SENSOR_V == SENSOR_AIRSPEED
//..
#elif SENSOR_V == SENSOR_ALTIMETER
        altitude_handle_data(ch);
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

    size_t n = Serial1.readBytes(sbuf, len);
    sbuf[n] = '\0';  // ✅ terminate

   // String x = (char *)sbuf;
   // Serial.println(x);

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
  Serial.println("  help                   - show help");
  Serial.println("  showwifi               - show stored Wi-Fi credentials");
  Serial.println("  wifi <ssid> <pass>     - store credentials and reboot");
  Serial.println("  clearwifi              - erase stored Wi-Fi");
  Serial.println("  simulate <true/false>  - set operation mode");
  Serial.println("  showsim                - show operation mode");
  Serial.println("  boot                   - reboot");
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

  if (cmd.startsWith("showsim")) {
    Serial.println(SIMULATE);
    return;
  }

  if (cmd.startsWith("simulate ")) {
    bool s1 = cmd.indexOf(' ');
    SIMULATE = s1;
    Serial.println(SIMULATE);
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
  Serial.begin(115200);

  uint32_t start = millis();
  while (!Serial && millis() - start < 5000) {
    delay(10);
  }

  Serial.print("v1.0 -> ");
  Serial.println(SENSOR);

  // Set up LED...
#ifdef USE_FastLED
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
#endif

  // Initialize Task Watchdog
  esp_task_wdt_config_t wdt_config = {
    .timeout_ms = WDT_TIMEOUT * 1000,
    .idle_core_mask = (1 << portNUM_PROCESSORS) - 1,  // watch both cores
    .trigger_panic = true                             // reset on timeout
  };
  // esp_task_wdt_init(&wdt_config);
  // Add loop() task to watchdog
  // esp_task_wdt_add(NULL);

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
  WiFi.disconnect(true);  // clear old connection
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
  //#ifdef NOTTEST
  server.begin();
  server.setNoDelay(true);
  //#endif

#if SENSOR_V == SENSOR_RADAR
  Serial1.begin(9600, SERIAL_8N1, 5, 4);  // Baud, format, RX pin, TX pin (check your board!)
  setupRADAR();
#elif SENSOR_V == SENSOR_TRANSPONDER
  Serial1.begin(9600, SERIAL_8N1, 5, 4);  // Baud, format, RX pin, TX pin (check your board!)
  setupTRANSPONDER();
#elif SENSOR_V == SENSOR_imu
//  imu_setup();
#elif SENSOR_V == SENSOR_ALTIMETER
  altimeter_setup();
#elif SENSOR_V == SENSOR_AIRSPEED
  airspeed_setup();
#endif

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
  int tx = millis();
  static bool lo = false;

  static int sleep = 0;
  bool hasclient = false;

  if (wifiMulti.run() != WL_CONNECTED) {
    Serial.println("WiFi lost!");
    delay(500);
    //    esp_task_wdt_reset();
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
    if (tx - td > 2500) {
      td = tx;
      send_ssdp_blind();
    }
  }

  handleSsdpReceive();
  acceptNewClients();

  if(SIMULATE == true){
    handleTelnetToSerial(active);
    handleSerialToTelnet();
  }
  else{
    #if SENSOR_V == SENSOR_RADAR
    loopRADAR();
    #elif SENSOR_V == SENSOR_TRANSPONDER
    loopTRANSPONDER();
    handleTelnetToSerial(active);
    handleSerialToTelnet();
    delay(100);
    #elif SENSOR_V == SENSOR_IMU
    handleTelnetToSerial(active);
    imu_loop();
    #elif SENSOR_V == SENSOR_ALTIMETER
    handleTelnetToSerial(active);
    altimeter_loop();
    #elif SENSOR_V == SENSOR_AIRSPEED
    airspeed_loop();
    #endif
  }

#ifdef USE_FastLED
  static int flash = 0;
  if (tx - flash > 500) {
    flash = tx;

    if (lo == true) {
      //  leds[0] = CRGB::Red; FastLED.show(); delay(500);
      leds[0] = CRGB::Green;
      FastLED.show();
    } else {
      //  leds[0] = CRGB::Blue; FastLED.show(); delay(500);
      leds[0] = 0;
      FastLED.show();
    }
    lo = !lo;
  }
#endif

  //  esp_task_wdt_reset();
}