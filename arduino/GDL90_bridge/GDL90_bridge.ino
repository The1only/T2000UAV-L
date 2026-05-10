/**
 * @file gdl90_forwarder.cpp
 * @brief ESP32 GDL90 UDP ↔ UART forwarder with Wi-Fi multi-AP support
 *
 * This firmware supports two build-time roles:
 *
 * - GDL90_SIDE:
 *     Receives GDL90 over UDP and forwards raw bytes to UART.
 *
 * - NON-GDL90_SIDE:
 *     Receives raw bytes over UART and forwards them as UDP packets.
 *
 * Wi-Fi credentials are loaded from NVS if present, otherwise a list
 * of fallback APs is tried using WiFiMulti.
 *
 * @note
 * UART data is forwarded as-is and is NOT frame-aligned to GDL90 (0x7E).
 * This may cause frame fragmentation and should be improved if strict
 * GDL90 compliance is required.
 *
 * @author
 * (c) 9Tek AS
 */

#include <WiFi.h>
#include <WiFiUdp.h>
#include <WiFiMulti.h>
#include <Wire.h>
#include <Preferences.h>
#include <U8g2lib.h>

// -----------------------------------------------------------------------------
// Build-time role selection
// -----------------------------------------------------------------------------

#define GDL90_SIDE   ///< Define for the ESP32 connected directly to GDL90 source

// -----------------------------------------------------------------------------
// Configuration constants
// -----------------------------------------------------------------------------

static constexpr uint16_t GDL90_PORT = 4000;   ///< UDP port for GDL90 traffic

/// Destination IP for UDP forwarding (unicast recommended)
static IPAddress DEST_IP(192, 168, 1, 255);

// -----------------------------------------------------------------------------
// NVS (Preferences) storage
// -----------------------------------------------------------------------------

static Preferences prefs;                 ///< NVS handle
static constexpr char PREF_NAMESPACE[] = "wifi";
static constexpr char KEY_SSID[] = "ssid";
static constexpr char KEY_PASS[] = "pass";

static bool   hasStoredWifi = false;       ///< True if NVS credentials exist
static String storedSsid;                  ///< Stored SSID
static String storedPass;                  ///< Stored password

// -----------------------------------------------------------------------------
// Wi-Fi and networking
// -----------------------------------------------------------------------------

static WiFiUDP   udp;                      ///< UDP socket
static WiFiMulti wifiMulti;                ///< Multi-AP manager

// Fallback AP list
// -----------------------------------------------------------------------------
// Wi-Fi credentials (compile-time defaults)
// -----------------------------------------------------------------------------
/**
 * @brief Wi-Fi access point credentials
 */
struct WiFiCred {
  const char* ssid;
  const char* pass;
};

#ifdef GDL90_SIDE
static const WiFiCred wifiCreds[] = {
  { "SkyEcho_0431",   "" },
};
#else
static const WiFiCred wifiCreds[] = {
  { "Hvattum",        "Jordvarme@2023@" },
  { "Altibox177449",  "HheX9Xac" },
  { "Aeros2",         "Terjenilsen1" },
};
#endif

static const size_t WIFI_CRED_COUNT =
    sizeof(wifiCreds) / sizeof(wifiCreds[0]);
// -----------------------------------------------------------------------------
// Display (OLED)
// -----------------------------------------------------------------------------

static constexpr uint8_t SDA_PIN_D = 5;
static constexpr uint8_t SCL_PIN_D = 6;

U8G2_SSD1306_72X40_ER_F_HW_I2C u8g2(
    U8G2_R0, U8X8_PIN_NONE);

// -----------------------------------------------------------------------------
// Buffers
// -----------------------------------------------------------------------------

static uint8_t udpBuffer[512];   ///< UDP / UART transfer buffer
static String  serialLine;       ///< CLI input buffer

// -----------------------------------------------------------------------------
// Forward declarations
// -----------------------------------------------------------------------------

void loadStoredCredentials();
void saveCredentials(const String&, const String&);
void clearCredentials();
void addAccessPoints();
bool connectWiFi();
bool handleSerialConfig();
bool processSerialCommand(const String&);
void printHelp();
void WiFiEvent(WiFiEvent_t, WiFiEventInfo_t);

// -----------------------------------------------------------------------------
// Setup
// -----------------------------------------------------------------------------

void setup() {
  Serial.begin(115200);
  Serial1.begin(115200, SERIAL_8N1, 20, 21);

#ifdef GDL90_SIDE
  Serial.println("\nGDL90 Forwarder GDL90 side v1.0");
#else
  Serial.println("\nGDL90 Forwarder Client side v1.0");
#endif

  Wire.begin(SDA_PIN_D, SCL_PIN_D);
  u8g2.begin();
  u8g2.setFont(u8g2_font_ncenB08_tr);

  u8g2.clearBuffer();
  u8g2.drawStr(0, 10, "(c) 9Tek AS");
#ifdef GDL90_SIDE
  u8g2.drawStr(0, 20, "GDL90 v1.0a");
#else
  u8g2.drawStr(0, 20, "GDL90 v1.0b");
#endif
  u8g2.sendBuffer();

  loadStoredCredentials();
  addAccessPoints();

  Serial.println("Hit any key to enter config...");
  for (uint32_t t = millis(); millis() - t < 5000;) {
    if (Serial.available()) {
      while (!handleSerialConfig()) {}
      break;
    }
    delay(1);
  }

  WiFi.onEvent(WiFiEvent);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true);
  delay(100);

  if (!connectWiFi()) {
    Serial.println("WiFi failed, rebooting...");
    ESP.restart();
  }

  #ifdef GDL90_SIDE
    // RX socket (GDL90 input)
    udp.begin(GDL90_PORT);
    Serial.printf("Listening UDP port %d (GDL90)\n", GDL90_PORT);
  #else
    // TX socket (forward to EFB / Mac)
    udp.begin(GDL90_PORT);
  #endif
}

// -----------------------------------------------------------------------------
// Main loop
// -----------------------------------------------------------------------------

void loop() {

#ifdef GDL90_SIDE
  int packetSize = udp.parsePacket();
  if (packetSize > 0) {
    int len = udp.read(udpBuffer, sizeof(udpBuffer));
    if (len > 0) {
      Serial1.write(udpBuffer, len);
      Serial.println(len);
    }
  }
#else
  if (Serial1.available()) {
    size_t len = Serial1.available();
    if (len > sizeof(udpBuffer)) len = sizeof(udpBuffer);

    size_t n = Serial1.readBytes(udpBuffer, len);
    if (n > 0) {
      udp.beginPacket(DEST_IP, GDL90_PORT);
      udp.write(udpBuffer, n);
      udp.endPacket();
      Serial.println(n);
    }
  }
#endif
}

// -----------------------------------------------------------------------------
// Serial CLI
// -----------------------------------------------------------------------------

void printHelp() {
  Serial.println("Commands:");
  Serial.println("  help");
  Serial.println("  wifi <ssid> <pass>");
  Serial.println("  showwifi");
  Serial.println("  clearwifi");
  Serial.println("  reboot");
  Serial.println("  boot");
}

bool processSerialCommand(const String& line) {
  String cmd = line;
  cmd.trim();

  if (cmd == "help") {
    printHelp();
  } else if (cmd == "reboot") {
    ESP.restart();
  } else if (cmd == "boot") {
    return true;
  } else if (cmd == "showwifi") {
    if (hasStoredWifi) {
      Serial.printf("SSID: %s\nPASS: %s\n",
                    storedSsid.c_str(),
                    storedPass.c_str());
    } else {
      Serial.println("No stored Wi-Fi");
    }
  } else if (cmd == "clearwifi") {
    clearCredentials();
  } else if (cmd.startsWith("wifi ")) {
    int p = cmd.indexOf(' ', 5);
    if (p < 0) return false;
    saveCredentials(cmd.substring(5, p),
                    cmd.substring(p + 1));
    Serial.println(cmd.substring(5, p) + " " + cmd.substring(p + 1));
    ESP.restart();
  } else {
    printHelp();
  }
  return false;
}

bool handleSerialConfig() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n') {
      bool ret = processSerialCommand(serialLine);
      serialLine.clear();
      return ret;
    } else if (c != '\r') {
      serialLine += c;
    }
  }
  return false;
}

// -----------------------------------------------------------------------------
// Wi-Fi helpers
// -----------------------------------------------------------------------------

void WiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info) {
  if (event == ARDUINO_EVENT_WIFI_STA_DISCONNECTED) {
    Serial.printf("WiFi disconnected (reason %d)\n",
                  info.wifi_sta_disconnected.reason);
  }
}

void loadStoredCredentials() {
  prefs.begin(PREF_NAMESPACE, true);
  storedSsid = prefs.getString(KEY_SSID);
  storedPass = prefs.getString(KEY_PASS);
  prefs.end();

  hasStoredWifi = storedSsid.length() > 0;
}

void saveCredentials(const String& ssid, const String& pass) {
  prefs.begin(PREF_NAMESPACE, false);
  prefs.putString(KEY_SSID, ssid);
  prefs.putString(KEY_PASS, pass);
  prefs.end();
}

void clearCredentials() {
  prefs.begin(PREF_NAMESPACE, false);
  prefs.clear();
  prefs.end();
  hasStoredWifi = false;
}

void addAccessPoints() {
  if (hasStoredWifi){
    wifiMulti.addAP(storedSsid.c_str(), storedPass.c_str());
  }

  for (size_t i = 0; i < WIFI_CRED_COUNT; ++i) {
    wifiMulti.addAP(wifiCreds[i].ssid, wifiCreds[i].pass);
  }
}

bool connectWiFi() {
  Serial.println("Connecting WiFi...");
  for (int i = 0; i < 20; ++i) {
    if (wifiMulti.run() == WL_CONNECTED) {

      Serial.print("Local IP : ");
      Serial.println(WiFi.localIP());

      Serial.print("SSID     : ");
      Serial.println(WiFi.SSID());

      Serial.print("AP / GW  : ");
      DEST_IP = WiFi.gatewayIP();
      DEST_IP[3] = 255;   // set last octet to .255
      Serial.println(DEST_IP);

      Serial.print("Subnet   : ");
      Serial.println(WiFi.subnetMask());

      u8g2.drawStr(0, 30, WiFi.SSID().c_str());
      u8g2.drawStr(0, 40, WiFi.localIP().toString().c_str());
      u8g2.sendBuffer();
      return true;
    }
    delay(400);
  }
  return false;
}
