#include <NimBLEDevice.h>

static const char* NUS_SERVICE_UUID = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E";
#define DEV_NAME "XIAO-S3-NUS"

void setup() {
  Serial.begin(115200);
  delay(200);

  NimBLEDevice::init(DEV_NAME);
  NimBLEDevice::setDeviceName(DEV_NAME);
  NimBLEDevice::setPower(ESP_PWR_LVL_P9);   // max TX power

  // Build primary ADV payload with FLAGS + NAME
  NimBLEAdvertisementData advData;
  advData.setFlags(BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP);
  advData.setName(DEV_NAME);

  // Optional scan response (you can omit it)
  NimBLEAdvertisementData scanResp;
  // scanResp.addTxPower(); // optional extra

  NimBLEAdvertising* adv = NimBLEDevice::getAdvertising();
  adv->setAdvertisementData(advData);
  adv->setScanResponseData(scanResp);

  // Make sure service UUID is present (goes into ADV by default)
  adv->addServiceUUID(NUS_SERVICE_UUID);

  // Start legacy, connectable advertising
  adv->start();
  Serial.println("[BLE] Advertising XIAO-S3-NUS with name + NUS UUID");
  pinMode(LED_BUILTIN, OUTPUT);
  
}

void loop() {
  static int run = 0;

  delay(1);

  if(++run == 1000){
    digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
  }
  if(run == 2000){
    digitalWrite(LED_BUILTIN, LOW);   // turn the LED off by making the voltage LOW
    run = 0;
  }

}