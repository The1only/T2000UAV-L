/*
 * ESP32 BLE UART (Nordic UART Service) for Qt BLE client (iOS + macOS compatible)
 * Works with ESP32-Arduino 3.x built-in NimBLE (no external NimBLE-Arduino needed)
 */

#include <NimBLEDevice.h>
#include <algorithm>
#include <vector>
#include <map>

// ---------- NUS UUIDs ----------
static const char* NUS_SERVICE_UUID = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E";
static const char* NUS_RX_CHAR_UUID = "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"; // Central -> ESP32 (Write/WriteNR)
static const char* NUS_TX_CHAR_UUID = "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"; // ESP32  -> Central (Notify)

// ---------- BLE objects ----------
NimBLEServer*         g_server  = nullptr;
NimBLEService*        g_service = nullptr;
NimBLECharacteristic* g_txChar  = nullptr;
NimBLECharacteristic* g_rxChar  = nullptr;
NimBLEAdvertising*    g_adv     = nullptr;

// Track centrals that enabled notifications on TX (conn handles)
static std::vector<uint16_t> g_subscribers;

// Cache per-connection MTU (filled from callbacks). Fallback is NimBLEDevice::getMTU()
static std::map<uint16_t, uint16_t> g_mtuByConn;

// ---------- Helpers ----------
static uint16_t mtuForConn(uint16_t connHandle) {
  auto it = g_mtuByConn.find(connHandle);
  if (it != g_mtuByConn.end()) return it->second;
  return NimBLEDevice::getMTU(); // global/default if unknown
}

static void bleSendToAll(const uint8_t* data, size_t len) {
  if (g_subscribers.empty() || len == 0) return;

  for (uint16_t ch : g_subscribers) {
    const uint16_t mtu = mtuForConn(ch);
    uint16_t payload = (mtu > 3 ? (mtu - 3) : 20);
    if (payload < 20) payload = 20; // conservative minimum

    size_t off = 0;
    while (off < len) {
      const size_t n = std::min((size_t)payload, len - off);
      g_txChar->setValue(data + off, n);
      g_txChar->notify(ch);
      off += n;
      // If you see drops on iOS, consider a tiny delay:
      // delay(1);
    }
  }
}

// ---------- Callbacks ----------
class RxCallbacks : public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic* ch, NimBLEConnInfo& connInfo) override {
    const std::string v = ch->getValue(); // raw bytes
    if (!v.empty()) {
      Serial.print("[BLE RX ");
      Serial.print(connInfo.getConnHandle());
      Serial.print("] ");
      Serial.write(reinterpret_cast<const uint8_t*>(v.data()), v.size());
      Serial.println();
    }
  }
};

class TxCallbacks : public NimBLECharacteristicCallbacks {
  void onSubscribe(NimBLECharacteristic* /*chr*/, NimBLEConnInfo& connInfo, uint16_t subValue) override {
    const uint16_t ch = connInfo.getConnHandle();
    const bool enabled = (subValue != 0);

    auto it = std::find(g_subscribers.begin(), g_subscribers.end(), ch);
    if (enabled) {
      if (it == g_subscribers.end()) g_subscribers.push_back(ch);
      g_mtuByConn[ch] = connInfo.getMTU();
      Serial.printf("[BLE] Notifications enabled by %u (mtu=%u)\n", ch, connInfo.getMTU());
    } else {
      if (it != g_subscribers.end()) g_subscribers.erase(it);
      Serial.printf("[BLE] Notifications disabled by %u\n", ch);
    }
  }
};

class ServerCallbacks : public NimBLEServerCallbacks {
  void onConnect(NimBLEServer* /*s*/, NimBLEConnInfo& connInfo) override {
    const uint16_t ch = connInfo.getConnHandle();
    g_mtuByConn[ch] = connInfo.getMTU();
    Serial.printf("[BLE] Connected: handle=%u mtu=%u\n", ch, connInfo.getMTU());
  }
  void onDisconnect(NimBLEServer* /*s*/, NimBLEConnInfo& connInfo, int reason) override {
    const uint16_t ch = connInfo.getConnHandle();
    Serial.printf("[BLE] Disconnected: handle=%u reason=%d\n", ch, reason);
    g_subscribers.erase(std::remove(g_subscribers.begin(), g_subscribers.end(), ch), g_subscribers.end());
    g_mtuByConn.erase(ch);
    NimBLEDevice::startAdvertising(); // resume advertising
  }
  void onMTUChange(uint16_t mtu, NimBLEConnInfo& connInfo) override {
    const uint16_t ch = connInfo.getConnHandle();
    g_mtuByConn[ch] = mtu;
    Serial.printf("[BLE] MTU changed: handle=%u mtu=%u (payload=%u)\n",
                  ch, mtu, (mtu > 3 ? mtu - 3 : 0));
  }
};

// ---------- Arduino setup/loop ----------
void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("\nESP32 BLE UART (NUS) — XIAO S3 / Core 3.x");

  // Initialize BLE stack
  NimBLEDevice::init("T2000-transponder");     // GAP name
  NimBLEDevice::setDeviceName("T2000-transponder");
  NimBLEDevice::setPower(ESP_PWR_LVL_P9);      // max TX power
  NimBLEDevice::setMTU(247);                   // request larger MTU (central negotiates actual)

  // Server + service
  g_server = NimBLEDevice::createServer();
  g_server->setCallbacks(new ServerCallbacks());

  g_service = g_server->createService(NUS_SERVICE_UUID);

  // TX: Notify
  g_txChar = g_service->createCharacteristic(
      NUS_TX_CHAR_UUID, NIMBLE_PROPERTY::NOTIFY);
  g_txChar->setCallbacks(new TxCallbacks());

  // RX: Write / Write Without Response
  g_rxChar = g_service->createCharacteristic(
      NUS_RX_CHAR_UUID, NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR);
  g_rxChar->setCallbacks(new RxCallbacks());

  g_service->start();

  // --- Advertising setup ---
  g_adv = NimBLEDevice::getAdvertising();

  // Primary ADV: include flags + (optionally) service UUID
  NimBLEAdvertisementData advData;
  advData.setFlags(BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP);
  // Keep the service UUID in the advertisement so CoreBluetooth can match quickly
  g_adv->addServiceUUID(NUS_SERVICE_UUID);
  g_adv->setAdvertisementData(advData);

  // Scan response: put the device name here (CoreBluetooth often expects it)
  NimBLEAdvertisementData scanResp;
  scanResp.setName("T2000-transponder");
  g_adv->setScanResponseData(scanResp);

  // Optional intervals (comment out if your NimBLE lacks these)
  // g_adv->setMinInterval(0x0030);  // ~30 ms
  // g_adv->setMaxInterval(0x0060);  // ~60 ms

  g_adv->start();

  Serial.println("[BLE] Advertising \"T2000-transponder\" with NUS service.");
  pinMode(LED_BUILTIN, OUTPUT);

}

void loop() {
  static int run = 0;

  // Read from USB Serial and forward to BLE subscribers
  static uint8_t buf[256];
  size_t n = 0;

  while (Serial.available() && n < sizeof(buf)) {
    buf[n++] = static_cast<uint8_t>(Serial.read());
    if (!Serial.available()) delay(2); // small batch delay
  }

  if (n > 0) {
    bleSendToAll(buf, n);

    // Print just the bytes we read, as hex
    for (size_t i = 0; i < n; ++i) {
      if (buf[i] < 0x10) Serial.print('0');
      Serial.print(buf[i], HEX);
      Serial.print(' ');
    }
    Serial.println();
  }

  // Periodic test message + LED pulse
  if (++run == 1000) {
  //  const char msg[] = "TERJE";
  //  bleSendToAll(reinterpret_cast<const uint8_t*>(msg), sizeof(msg) - 1);
#if defined(LED_BUILTIN)
    digitalWrite(LED_BUILTIN, HIGH);
#endif
  } else if (run == 2000) {
#if defined(LED_BUILTIN)
    digitalWrite(LED_BUILTIN, LOW);
#endif
    run = 0;
  }

  delay(1);
}