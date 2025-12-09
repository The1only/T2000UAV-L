/*
 * ESP32 Telnet bridge + multicast announcer + IMU simulator
 */

#include <Adafruit_NeoPixel.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiUdp.h>

#define SIMULATE
#define SENSOR "IMU"

// ------------------------ Hardware / LED ------------------------
// ~50 Hz main loop
static const uint16_t PERIOD_MS = 20;

// ------------------------ WiFi / Telnet ------------------------

WiFiMulti wifiMulti;

// how many clients should be able to telnet to this ESP32
static const bool USE_SERIAL = false;
#define MAX_SRV_CLIENTS 1

// Access points
const char *ssid1     = "Altibox177449";
const char *pass1     = "HheX9Xac";

const char *ssid2     = "Hvattum";
const char *pass2     = "Jordvarme@2023@";

const char *ssid3     = "Aeros2";
const char *pass3     = "Terjenilsen1";

const char *ssid0     = "9Tek_printer";
const char *pass0     = "didiinne";

// Telnet server
WiFiServer server(23);
WiFiClient serverClients[MAX_SRV_CLIENTS];

// ------------------------ Multicast ------------------------

WiFiUDP udp;
const IPAddress MULTICAST_ADDR(239, 255, 0, 1);
const uint16_t  MULTICAST_PORT = 4210;

unsigned long lastAnnounce           = 0;
const unsigned long ANNOUNCE_INTERVAL_MS = 5000; // currently gated by "Boot" logic

static int received_ping = 0;   // set when we receive "Boot"
bool active = false;

// ------------------------ IMU Packet Helpers ------------------------

// clamp to int16 range
int16_t clamp16(long v) {
  if (v >  32767) return  32767;
  if (v < -32768) return -32768;
  return (int16_t)v;
}

// scale degrees -> protocol int16 (±180°)
int16_t angle_to_q15(float deg) {
  return clamp16((long)(deg * 32768.0f / 180.0f));
}

// scale dps -> protocol int16 (±2000 dps)
int16_t dps_to_q15(float dps) {
  return clamp16((long)(dps * 32768.0f / 2000.0f));
}

// scale g -> protocol int16 (±16 g)
int16_t g_to_q15(float g) {
  return clamp16((long)(g * 32768.0f / 16.0f));
}

// Temperature encoding (raw from Celsius)
// T = ((TH << 8) | TL) / 340 + 36.53  => raw ≈ (T - 36.53) * 340
uint16_t make_temp_raw(float celsius = 25.0f) {
  float raw = (celsius - 36.53f) * 340.0f;
  long r    = (long)(raw);
  if (r < 0)      r = 0;
  if (r > 65535)  r = 65535;
  return (uint16_t)r;
}

// Build and send a standard 11-byte WIT packet to all telnet clients
void sendPacket(uint8_t pid, int16_t x, int16_t y, int16_t z, uint16_t tRaw) {
  uint8_t pkt[11];

  pkt[0] = 0x55;
  pkt[1] = pid;
  pkt[2] = (uint8_t)(x & 0xFF);
  pkt[3] = (uint8_t)((x >> 8) & 0xFF);
  pkt[4] = (uint8_t)(y & 0xFF);
  pkt[5] = (uint8_t)((y >> 8) & 0xFF);
  pkt[6] = (uint8_t)(z & 0xFF);
  pkt[7] = (uint8_t)((z >> 8) & 0xFF);
  pkt[8] = (uint8_t)(tRaw & 0xFF);
  pkt[9] = (uint8_t)((tRaw >> 8) & 0xFF);

  uint16_t sum = 0;
  for (int i = 0; i < 10; ++i)
    sum += pkt[i];
  pkt[10] = (uint8_t)(sum & 0xFF);

  for (int i = 0; i < MAX_SRV_CLIENTS; i++) {
    if (serverClients[i] && serverClients[i].connected()) {
      serverClients[i].write(pkt, sizeof(pkt));
    }
  }
}

// ------------------------ WiFi helpers ------------------------

void addAccessPoints() {
  wifiMulti.addAP(ssid0, pass0);
  wifiMulti.addAP(ssid1, pass1);
  wifiMulti.addAP(ssid2, pass2);
  wifiMulti.addAP(ssid3, pass3);
}

bool connectWiFi() {
  Serial.println("Connecting WiFi...");
  for (int loops = 20; loops > 0; --loops) {
    if (wifiMulti.run() == WL_CONNECTED) {
      Serial.println();
      Serial.print("WiFi connected. IP address: ");
      Serial.println(WiFi.localIP());

      Serial.print("Connected SSID: ");
      Serial.println(WiFi.SSID());

      Serial.print("Access Point BSSID: ");
      Serial.println(WiFi.BSSIDstr());

      Serial.print("RSSI: ");
      Serial.println(WiFi.RSSI());
      return true;
    } else {
      Serial.print("Retry in ");
      Serial.print(loops);
      Serial.println(" x 400ms");
      delay(400);
    }
  }
  return false;
}

// ------------------------ Multicast helpers ------------------------

void startMulticast() {
  // ESP32 core 3.x: beginMulticast(group, port)
  if (udp.beginMulticast(MULTICAST_ADDR, MULTICAST_PORT)) {
    Serial.print("Multicast started on ");
    Serial.print(MULTICAST_ADDR);
    Serial.print(':');
    Serial.println(MULTICAST_PORT);

    // Initial announcement
    udp.beginPacket(MULTICAST_ADDR, MULTICAST_PORT);
    udp.print("ESP32 online, IP=");
    udp.print(WiFi.localIP());
    udp.endPacket();
  } else {
    Serial.println("Failed to start multicast UDP");
  }
}

void sendMulticastAnnouncement() 
{
    // If we have received "Boot", send our multicast announcement
  if (received_ping > 0) {
    --received_ping;

    udp.beginPacket(MULTICAST_ADDR, MULTICAST_PORT);
    udp.print(SENSOR);
    udp.print(" ");
    udp.print(WiFi.localIP());
    udp.endPacket();

    Serial.print("WiFi Multicast. IP address: ");
    Serial.print(WiFi.localIP());
    Serial.print(" : ");
    Serial.println(MULTICAST_PORT);
  }
}

// returns true if we received something
bool handleMulticastReceive() {
  int packetSize = udp.parsePacket();
  if (packetSize <= 0)
    return false;

  char buf[256];
  int len = udp.read(buf, sizeof(buf) - 1);
  if (len <= 0)
    return false;

  buf[len] = '\0';

  IPAddress remote = udp.remoteIP();
  uint16_t rport   = udp.remotePort();

  Serial.print("Multicast received from ");
  Serial.print(remote);
  Serial.print(':');
  Serial.print(rport);
  Serial.print(" -> ");
  Serial.println(buf);

  if (!strncmp(buf, "Boot", 4)) {
    received_ping = 1;  // start sending our announcements
    Serial.println(" -> Ping received");
  }

  return true;
}

// ------------------------ Telnet helpers ------------------------

void acceptNewClients() {
  if (!server.hasClient())
    return;

  for (int i = 0; i < MAX_SRV_CLIENTS; i++) {
    // find free/disconnected spot
    if (!serverClients[i] || !serverClients[i].connected()) {
      if (serverClients[i]) {
        serverClients[i].stop();
      }
      serverClients[i] = server.available();
      if (!serverClients[i]) {
        Serial.println("available broken");
        return;
      }
      Serial.print("New client: ");
      Serial.print(i);
      Serial.print(' ');
      Serial.println(serverClients[i].remoteIP());

      if (serverClients[i] && serverClients[i].connected()) {
        Serial.println("Server OK...");
      }

      return;
    }
  }

  // No free slot -> reject
  WiFiClient tmp = server.available();
  if (tmp) {
    tmp.stop();
  }
}

void handleTelnetToSerial(bool &activeFlag) {
  for (int i = 0; i < MAX_SRV_CLIENTS; i++) {
    if (serverClients[i] && serverClients[i].connected() && serverClients[i].available()) {
      while (serverClients[i].available()) {
        int ch = serverClients[i].read();
        activeFlag = true;
        if (USE_SERIAL) {
          Serial1.write(ch);
        } else {
          Serial.write(ch);
        }
      }
    } 
    //else if (serverClients[i]) {
    //  serverClients[i].stop();
    // }
  }
}

void handleSerialToTelnet() {
  if (!USE_SERIAL) {
    return;
  }

  if (Serial1.available()) {
    size_t len = Serial1.available();
    uint8_t sbuf[256];
    if (len > sizeof(sbuf)) len = sizeof(sbuf);

    Serial1.readBytes(sbuf, len);
    Serial1.write((char *)sbuf); // local echo (optional)

    for (int i = 0; i < MAX_SRV_CLIENTS; i++) {
      if (serverClients[i] && serverClients[i].connected()) {
        serverClients[i].write(sbuf, len);
      }
    }
  }
}

// ------------------------ Setup & Loop ------------------------

void setup() {
  Serial.begin(9600);
  Serial.println();
  Serial.print("Booting and Connecting v1.0 -> ");
  Serial.println(SENSOR);

  addAccessPoints();

  if (!connectWiFi()) {
    Serial.println("WiFi connect failed, restarting...");
    delay(1000);
    ESP.restart();
  }

  startMulticast();

  if (USE_SERIAL) {
//    Serial1.begin(115200);
    Serial1.begin(9600);  // if you actually want 9600 for UART bridge
  }

  server.begin();
  server.setNoDelay(true);

  Serial.print("Ready! Use 'telnet ");
  Serial.print(WiFi.localIP());
  Serial.println(" 23' to connect");
}

void loop() {
  static bool     active = false;
  uint32_t t0 = millis();

  // basic "not connected" watchdog
  if (wifiMulti.run() != WL_CONNECTED) {
    Serial.println("WiFi not connected!");
    for (int i = 0; i < MAX_SRV_CLIENTS; i++) {
      if (serverClients[i]) {
        serverClients[i].stop();
      }
    }
//    delay(1000);
    return;
  }

  // (you can also use time-based ANNOUNCE_INTERVAL_MS here if you want)
  sendMulticastAnnouncement();

  // Multicast receive
  handleMulticastReceive();

  // Telnet handling
  acceptNewClients();
//  handleTelnetToSerial(active);
  handleSerialToTelnet();

  // IMU simulation when not using Serial1 as source
 // if (server.hasClient())
  {
 //   if (lastAnnounce+20 < t0) 
    {
      lastAnnounce = t0;

      float t = millis() * 0.001f;     // seconds

      float roll  = 60.0f * sinf(2.0f * 3.14159f * 0.2f * t);   // ±60°
      float pitch = 30.0f * sinf(2.0f * 3.14159f * 0.1f * t);   // ±30°
      float yaw   = fmodf(t * 20.0f, 360.0f) - 180.0f;          // -180..+180°

      int16_t r_q   = angle_to_q15(roll);
      int16_t p_q   = angle_to_q15(pitch);
      int16_t y_q   = angle_to_q15(yaw);
      uint16_t temp = make_temp_raw(25.0f);

      sendPacket(0x53, r_q, p_q, y_q, temp);  // Angle packet
    }
  }

#ifndef SIMULATE
  while (Serial1.available()) {
    static int state = 0;
    char inChar = (char)Serial1.read();

    switch(state){
      case 0:
      if(inChar == 0xFF) state = 1;
      break;
      case 1:
      if(inChar == 0xAA) state = 2;
      else state = 0;
      break;
      case 2:
      switch(inChar){
        case 0x63:
          Serial1.begin(15200);
          state = 0;
        break;
        case 0x64:
          Serial1.begin(9600);
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
      if(inChar == 0x34) active = true;
      else active = false;
      state = 4;
      break;
      case 4:
      state = 0;
      break;
      default: break;
    }
  }
#else
  delay(PERIOD_MS);
#endif

}