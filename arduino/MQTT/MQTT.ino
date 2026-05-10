#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiUdp.h>

// ---------- WiFi ----------
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

// ---------- Broker settings ----------
static const uint16_t MQTT_PORT = 1883;
static const int MAX_CLIENTS = 4;
static const int MAX_SUBS_PER_CLIENT = 8;
static const int MAX_TOPIC_LEN = 64;

WiFiServer server(MQTT_PORT);

struct Subscription {
  bool used = false;
  char topic[MAX_TOPIC_LEN] = {0};
};

struct ClientState {
  WiFiClient sock;
  bool inUse = false;
  bool connected = false;        // MQTT-level connected (after CONNECT)
  uint32_t lastSeenMs = 0;
  Subscription subs[MAX_SUBS_PER_CLIENT];
};

ClientState clients[MAX_CLIENTS];

// ---------- setup ----------
void setup() {
  Serial.begin(115200);
  delay(200);

  wifiMulti.addAP(ssid0, pass0);
  wifiMulti.addAP(ssid1, pass1);
  wifiMulti.addAP(ssid2, pass2);
  wifiMulti.addAP(ssid3, pass3);

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

//  WiFi.onEvent(WiFiEvent);
//  WiFi.mode(WIFI_STA);
//  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("WiFi connecting");
  while (wifiMulti.run() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("WiFi OK, IP: ");
  Serial.println(WiFi.localIP());

  server.begin();
  server.setNoDelay(true);
  Serial.printf("MQTT broker listening on %u\n", MQTT_PORT);
}

// ---------- loop ----------
void loop() {
  acceptNewClients();
  for (int i = 0; i < MAX_CLIENTS; i++) {
    serviceClient(i);
  }
  delay(2);
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

// ---------- Helpers ----------
static uint16_t readU16(const uint8_t* p) {
  return (uint16_t(p[0]) << 8) | uint16_t(p[1]);
}

// MQTT Remaining Length (variable encoding)
static bool readRemainingLength(WiFiClient& c, uint32_t& outLen) {
  outLen = 0;
  uint32_t multiplier = 1;
  for (int i = 0; i < 4; i++) {
    int b = c.read();
    if (b < 0) return false;
    outLen += (uint32_t(b & 127) * multiplier);
    if ((b & 128) == 0) return true;
    multiplier *= 128;
  }
  return false;
}

// write Remaining Length
static void writeRemainingLength(uint8_t* out, size_t& n, uint32_t len) {
  n = 0;
  do {
    uint8_t digit = len % 128;
    len /= 128;
    if (len > 0) digit |= 0x80;
    out[n++] = digit;
  } while (len > 0 && n < 4);
}

static bool topicMatches(const char* sub, const char* topic) {
  // minimal matching: exact or '+' wildcard for single level
  // Examples:
  //  sub: "a/+/c" matches "a/b/c"
  //  sub: "a/b"  matches "a/b"
  const char* s = sub;
  const char* t = topic;

  while (*s && *t) {
    if (*s == '+') {
      // skip one level in topic
      while (*t && *t != '/') t++;
      s++; // move past '+'
      if (*s == '/' && *t == '/') { s++; t++; }
      continue;
    }
    if (*s != *t) return false;
    s++; t++;
  }
  return (*s == '\0' && *t == '\0');
}

static void sendConnack(WiFiClient& c, uint8_t returnCode = 0x00) {
  // Fixed header: 0x20, RL=2
  uint8_t pkt[4] = {0x20, 0x02, 0x00, returnCode};
  c.write(pkt, sizeof(pkt));
}

static void sendPingresp(WiFiClient& c) {
  uint8_t pkt[2] = {0xD0, 0x00};
  c.write(pkt, sizeof(pkt));
}

static void sendSuback(WiFiClient& c, uint16_t packetId) {
  uint8_t pkt[5];
  pkt[0] = 0x90; // SUBACK
  pkt[1] = 0x03; // RL
  pkt[2] = uint8_t(packetId >> 8);
  pkt[3] = uint8_t(packetId & 0xFF);
  pkt[4] = 0x00; // granted QoS 0
  c.write(pkt, sizeof(pkt));
}

static bool addSubscription(ClientState& cs, const char* topic) {
  for (int i = 0; i < MAX_SUBS_PER_CLIENT; i++) {
    if (cs.subs[i].used && strncmp(cs.subs[i].topic, topic, MAX_TOPIC_LEN) == 0)
      return true; // already
  }
  for (int i = 0; i < MAX_SUBS_PER_CLIENT; i++) {
    if (!cs.subs[i].used) {
      cs.subs[i].used = true;
      strncpy(cs.subs[i].topic, topic, MAX_TOPIC_LEN - 1);
      cs.subs[i].topic[MAX_TOPIC_LEN - 1] = '\0';
      return true;
    }
  }
  return false;
}

// Publish QoS0 to a client
static void publishToClient(WiFiClient& c, const char* topic, const uint8_t* payload, uint32_t payloadLen) {
  uint16_t topicLen = (uint16_t)strlen(topic);
  uint32_t remLen = 2 + topicLen + payloadLen; // topic length field + topic + payload

  uint8_t rlEnc[4]; size_t rlN = 0;
  writeRemainingLength(rlEnc, rlN, remLen);

  // Fixed header: PUBLISH QoS0, no retain: 0x30
  c.write((uint8_t)0x30);
  c.write(rlEnc, rlN);

  // Topic
  uint8_t tlen[2] = { uint8_t(topicLen >> 8), uint8_t(topicLen & 0xFF) };
  c.write(tlen, 2);
  c.write((const uint8_t*)topic, topicLen);

  // Payload
  if (payloadLen > 0) c.write(payload, payloadLen);
}

static void broadcastPublish(const char* topic, const uint8_t* payload, uint32_t payloadLen, int fromIdx) {
  for (int i = 0; i < MAX_CLIENTS; i++) {
    if (!clients[i].inUse || !clients[i].connected) continue;
    if (!clients[i].sock.connected()) continue;

    bool match = false;
    for (int s = 0; s < MAX_SUBS_PER_CLIENT; s++) {
      if (!clients[i].subs[s].used) continue;
      if (topicMatches(clients[i].subs[s].topic, topic)) {
        match = true;
        break;
      }
    }
    if (match) {
      publishToClient(clients[i].sock, topic, payload, payloadLen);
    }
  }
}

// ---------- Packet handlers ----------
static void handleConnect(ClientState& cs, uint32_t remLen) {
  // We’ll read the whole remaining payload into a small buffer (connect packet is usually small)
  if (remLen > 512) { cs.sock.stop(); return; }
  static uint8_t buf[512];
  if (cs.sock.readBytes((char*)buf, remLen) != remLen) return;

  // Parse: Protocol Name, Level, Flags, Keep Alive, ClientId...
  // Protocol name length at buf[0..1]
  if (remLen < 10) { sendConnack(cs.sock, 0x01); return; }
  uint16_t pnLen = readU16(buf);
  if (2 + pnLen + 4 > remLen) { sendConnack(cs.sock, 0x01); return; }

  const char* proto = (const char*)(buf + 2);
  uint8_t level = buf[2 + pnLen];
  // uint8_t flags = buf[2 + pnLen + 1];
  // uint16_t keepAlive = readU16(buf + 2 + pnLen + 2);

  bool okProto = (pnLen == 4 && strncmp(proto, "MQTT", 4) == 0 && level == 4);
  if (!okProto) { sendConnack(cs.sock, 0x01); return; }

  cs.connected = true;
  cs.lastSeenMs = millis();
  sendConnack(cs.sock, 0x00);
}

static void handleSubscribe(ClientState& cs, uint32_t remLen) {
  if (remLen > 512) return;
  static uint8_t buf[512];
  if (cs.sock.readBytes((char*)buf, remLen) != remLen) return;

  if (remLen < 4) return;
  uint16_t packetId = readU16(buf);
  uint32_t pos = 2;

  // payload: repeated [topic filter (utf8 string), requested qos]
  while (pos + 3 <= remLen) {
    uint16_t tlen = readU16(buf + pos);
    pos += 2;
    if (pos + tlen + 1 > remLen) break;

    char topic[MAX_TOPIC_LEN];
    uint16_t copyLen = (tlen >= MAX_TOPIC_LEN) ? (MAX_TOPIC_LEN - 1) : tlen;
    memcpy(topic, buf + pos, copyLen);
    topic[copyLen] = '\0';
    pos += tlen;

    uint8_t reqQos = buf[pos++];
    (void)reqQos; // we only grant QoS0

    addSubscription(cs, topic);
  }

  sendSuback(cs.sock, packetId);
}

static void handlePublish(ClientState& cs, uint8_t header, uint32_t remLen, int fromIdx) {
  // QoS must be 0 for this minimal broker
  uint8_t qos = (header >> 1) & 0x03;
  if (qos != 0) {
    // consume bytes, ignore
    while (remLen--) cs.sock.read();
    return;
  }

  if (remLen > 1024) { // keep it small
    while (remLen--) cs.sock.read();
    return;
  }

  static uint8_t buf[1024];
  if (cs.sock.readBytes((char*)buf, remLen) != remLen) return;

  if (remLen < 2) return;
  uint16_t tlen = readU16(buf);
  if (2 + tlen > remLen || tlen >= MAX_TOPIC_LEN) return;

  char topic[MAX_TOPIC_LEN];
  memcpy(topic, buf + 2, tlen);
  topic[tlen] = '\0';

  uint32_t payloadPos = 2 + tlen;
  uint32_t payloadLen = remLen - payloadPos;

  broadcastPublish(topic, buf + payloadPos, payloadLen, fromIdx);
}

// ---------- Main loop ----------
static void acceptNewClients() {
  WiFiClient incoming = server.accept();
  if (!incoming) return;

  // find slot
  for (int i = 0; i < MAX_CLIENTS; i++) {
    if (!clients[i].inUse || !clients[i].sock.connected()) {
      clients[i].sock.stop();
      clients[i] = ClientState{};
      clients[i].sock = incoming;
      clients[i].inUse = true;
      clients[i].connected = false;
      clients[i].lastSeenMs = millis();
      Serial.printf("Client connected slot %d\n", i);
      return;
    }
  }

  // no slot
  incoming.stop();
  Serial.println("No client slots; rejected");
}

static void serviceClient(int idx) {
  ClientState& cs = clients[idx];
  if (!cs.inUse) return;

  if (!cs.sock.connected()) {
    cs.sock.stop();
    cs = ClientState{};
    return;
  }

  // Handle packets while available
  while (cs.sock.available() >= 2) {
    int h = cs.sock.read();
    if (h < 0) return;
    uint8_t header = (uint8_t)h;

    uint32_t remLen = 0;
    if (!readRemainingLength(cs.sock, remLen)) return;

    uint8_t packetType = header >> 4;
    cs.lastSeenMs = millis();

    switch (packetType) {
      case 1: // CONNECT
        handleConnect(cs, remLen);
        break;
      case 3: // PUBLISH
        handlePublish(cs, header, remLen, idx);
        break;
      case 8: // SUBSCRIBE
        handleSubscribe(cs, remLen);
        break;
      case 12: // PINGREQ
        // consume remaining (should be 0)
        while (remLen--) cs.sock.read();
        sendPingresp(cs.sock);
        break;
      case 14: // DISCONNECT
        while (remLen--) cs.sock.read();
        cs.sock.stop();
        cs = ClientState{};
        return;
      default:
        // consume and ignore
        while (remLen--) cs.sock.read();
        break;
    }
  }

  // Basic idle timeout cleanup (TCP might still be connected)
  const uint32_t now = millis();
  if (cs.inUse && (now - cs.lastSeenMs) > 120000) { // 2 minutes
    cs.sock.stop();
    cs = ClientState{};
  }
}
