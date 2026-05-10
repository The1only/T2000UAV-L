//#define T10_V20
//#define BTSerial
//#define Bluetooth
#define simulate
bool Debug = true;

#include <SPI.h>
#include <Wire.h>

#ifdef Bluetooth
  #ifdef BTSerial
  #include "BluetoothSerial.h"
  #else
  #include <BLEDevice.h>
  #include <BLEUtils.h>
  #include <BLEServer.h>
  #include <BLE2902.h>
  #endif
#endif

#include <stdio.h>
#include "string.h"

#ifdef Bluetooth
  #ifdef BTSerial
  //#define bluetoothModuleSerial Serial1
  // Create a BluetoothSerial object
  // Serial port that the bluetooth module is connected
  // Verbose mode: true
  //BluetoothSerial blueSerial(bluetoothModuleSerial, true);

  // BluetoothSerial SerialBT();
  boolean confirmRequestPending = true;

  void BTConfirmRequestCallback(uint32_t numVal) {
    confirmRequestPending = true;
    Serial.println(numVal);
  // SerialBT.confirmReply(true);
  }

  void BTAuthCompleteCallback(boolean success) {
    confirmRequestPending = false;
    if (success) {
      Serial.println("Pairing success!!");
    } else {
      Serial.println("Pairing failed, rejected by user!!");
    }
  }
  #else
  #include <BLEDevice.h>
  #include <BLEUtils.h>
  #include <BLEServer.h>
  #include <BLE2902.h>

  // See the following for generating UUIDs:
  // https://www.uuidgenerator.net/

  #define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
  #define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

  BLEServer* pServer = NULL;
  BLECharacteristic* pCharacteristic = NULL;
  BLEService *pService = NULL;
  //bool deviceConnected = false;
  bool confirmRequestPending = true;

  class MyServerCallbacks: public BLEServerCallbacks {
      void onConnect(BLEServer* pServer) {
        confirmRequestPending = false;
        Serial.println("Pairing success!!");
      };

      void onDisconnect(BLEServer* pServer) {
        confirmRequestPending = true;
      }
  };

  class MyCallbacks: public BLECharacteristicCallbacks {
    void onRead(BLECharacteristic *pCharacteristic) {
      pCharacteristic->setValue("Hello World!");
    }
    
    void onWrite(BLECharacteristic *pCharacteristic) {
  //   std::string value = pCharacteristic->getValue();
    }
  };
  #endif
#else
boolean confirmRequestPending = true;
#endif

//define the pins used by the LoRa transceiver module
#define OLED_SDA 4
#define SCK 5
#if defined(T10_V20)
#define RXD2 22 
#define TXD2 21 
#else
#define RXD2 17
#define TXD2 12
#endif
#define RST 14
#define OLED_SCL 15
#define OLED_RST 16
#define SS 18
#define MISO 19
#if defined(T10_V20)
#define BUZZER 25
#else
#define BUZZER 27
#endif
#define DIO0 26
#define MOSI 27

//433E6 for Asia
//866E6 for Europe
//915E6 for North America
#define BAND 866E6

//OLED pins
//#define SCREEN_WIDTH 128  // OLED display width, in pixels
//#define SCREEN_HEIGHT 64  // OLED display height, in pixels

bool Dodisplay = false;
long send = 0;
char buff[100];
bool ident = false;
int mode = 4;
int alt = 0;
int squak = 7000;
int pos = 30;
bool Annunciator = false;
bool ping = false;
bool hwcheck = false;

uint8_t inputbuff[100];
int inputpos = 0;

bool oldDeviceConnected = false;

//define USE_PIN // Uncomment this to use PIN during pairing. The pin is specified on the line below
#if defined(USE_PIN)
const char *pin = "1234";  // Change this to more secure PIN.
#endif

//--------------------------------------------
#define NOTE_DL2 165
#define NOTE_DH5 882
void playSound(int tone) {        
//  M5.Speaker.tone(NOTE_DH5);  // Set the speaker to ring at 661Hz for 200ms.
  delay(tone);
//  M5.Speaker.end();
}

//--------------------------------------------
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);

//  pinMode(RXD2, INPUT_PULLUP);
//  pinMode(TXD2, OUTPUT);
//  Serial1.begin(9600, SERIAL_8N1, RXD2, TXD2);

  Serial.begin(115200);
  Serial.println("Transponder v1.0");

#ifdef Bluetooth
  #ifdef BTSerial
    bluetoothModuleSerial.begin(9600);
    SerialBT.enableSSP();
  #ifdef USE_PIN
    SerialBT.setPin(pin);
    Serial.println("Using PIN");
  #endif
    SerialBT.onConfirmRequest(BTConfirmRequestCallback);
    SerialBT.onAuthComplete(BTAuthCompleteCallback);
    SerialBT.begin("Transponder");  //Bluetooth device name
  #else
    BLEDevice::init("Transponder");
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());
    pService = pServer->createService(SERVICE_UUID);
    pCharacteristic = pService->createCharacteristic(
                                          CHARACTERISTIC_UUID,
                                          BLECharacteristic::PROPERTY_READ |
                                          BLECharacteristic::PROPERTY_WRITE |
                                          BLECharacteristic::PROPERTY_NOTIFY |
                                          BLECharacteristic::PROPERTY_INDICATE
                                        );

  //  pCharacteristic->setValue("Hello Transponder...");

    pCharacteristic->setCallbacks(new MyCallbacks());
    pCharacteristic->addDescriptor(new BLE2902());
    pService->start();

    BLEAdvertising *pAdvertising = pServer->getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
    pAdvertising->setMinPreferred(0x12);
    //BLEDevice::startAdvertising();
    pAdvertising->start();
  #endif
#endif

  Serial.println("The device started, now you can pair it with bluetooth!");
  Serial.println("9Tek AS");

  Serial.println("...booting...");
  Serial.println("Waiting a client connection to notify...");

}

//--------------------------------------------------
int wait = 0;
int state = 0;
int m = 2;

int bt_inn  = 0;
int old_bt_inn  = 0;
int m_bt = 2;
int pos_bt = 0;

int ser_inn = 0;
int old_ser_inn = 0;
int m_ser = 2;
int pos_ser = 0;

int timout = 10;

char output_buffer[50];
int output_buffer_head=0;

char input_buffer[50];
int input_buffer_head=0;
int input_buffer_tail=0;

void write_to_transponder(char r)
{
//  Serial1.write(r);
  if (Debug) Serial.write(r);
  
#ifdef simulate
  output_buffer[output_buffer_head++]=r;
  if(output_buffer_head >= sizeof(output_buffer)) output_buffer_head=0;

  if(r < 0x20){
    switch(output_buffer[0]){
      case 'z':
        sprintf(input_buffer,"T2000U xx-x.x.x\n");
      break;
      case 'c':
        sprintf(input_buffer,"c=%d\n",squak);
      break;
      case 'a':
        sprintf(input_buffer,"a=%dF\n",alt);
      break;
      case 'i':
        if(output_buffer[2] == '?'){
          sprintf(input_buffer,"i=%d\n",(ident==true)?"true":"false");
        }
        else{
                    
        }
      break;
      case 's':
        sprintf(input_buffer,"s=%s\n","t");
      break;
      case 'r':
        sprintf(input_buffer,"r=%s\n",(Annunciator==true)?"Y":"N");
      break;
      case 'p':
        sprintf(input_buffer,"p=%s\n",(hwcheck==true)?"Y":"N");
      break;
      default:
      break;
    }
    output_buffer_head=strlen(input_buffer);
  }
#endif
}

void send_to_transponder(const char *x)
{
  int size = strlen(x);
  for(int i=0; i < size; i++)
  {
    write_to_transponder(*x);
    output_buffer[output_buffer_head++]=*x++;
    if(output_buffer_head >= sizeof(output_buffer)) output_buffer_head=0;
  }
}

void loop() 
{
  // While we are not connected... Get some values to display in the OLED...
  if (confirmRequestPending) 
  {
    if (wait++ > 80000) {
      wait = 0;

      switch (state) {
        case 0:
          send_to_transponder("v=1\r\n");
          break;
        case 1:
          send_to_transponder("z=?\r\n");
          break;
        case 2:
          send_to_transponder("c=?\r\n");
       //   SerialBT.println("c=7003\r\n");
          break;
        case 3:
          send_to_transponder("a=?\r\n");
          break;
        case 4:
          send_to_transponder("i=?\r\n");
          break;
        case 5:
          send_to_transponder("s=?\r\n");
          break;
        case 6:
          send_to_transponder("r=y\r\n");
          break;
      }
      if (++state > 6) state = 2;
    }
  }

  char x[50];
  uint8_t x2[50];
  char x3[50];

  if (send++ > 50000) {
    static bool toggle = false;
    send = 0;
    Dodisplay = true;

    if(toggle == false){
      digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
      toggle = true;
    }else{
      digitalWrite(LED_BUILTIN, LOW);   // turn the LED off by making the voltage LOW
      toggle = false;
    }
  
    // If we loose connection to bluetooth...
    if( timout++ > 10){
      timout= 10;
      confirmRequestPending = true;
    }
  }

#ifdef simulate
  if (input_buffer_tail != input_buffer_head) {
    inputbuff[inputpos] = input_buffer[input_buffer_tail];
    if(input_buffer[input_buffer_tail] < 0x20){
      input_buffer_tail = 0;
      input_buffer_head = 0;
    }
    else{
      if(++input_buffer_tail >= sizeof(input_buffer)) input_buffer_tail=0;
    }
#else
  if (Serial1.available()) {
    inputbuff[inputpos] = Serial1.read();
#endif
    ser_inn++;
    if (Debug) Serial.write(inputbuff[inputpos]);

    if (inputbuff[inputpos] == '*')
    {
      ping = true;
      //SerialBT.println("*");
      playSound(NOTE_DL2);
    }
    else{
      inputpos += 1;

      if (inputbuff[inputpos - 1] < 0x1F || inputpos >= 20) {
        if (inputpos >= 3) {
          inputbuff[inputpos - 1] = '\n';
          inputbuff[inputpos] = 0;
          if (!confirmRequestPending) 
          {
            //SerialBT.println((char *)inputbuff);
          }
          if (Debug) Serial.println((char *)inputbuff);
          if (inputbuff[0] == 'i' && inputbuff[1] == '=' && inputbuff[2] == '1') { ident = true; }
          if (inputbuff[0] == 'i' && inputbuff[1] == '=' && inputbuff[2] == '0') { ident = false; }
          if (inputbuff[0] == 's' && inputbuff[1] == '=' && inputbuff[2] == 'o') { mode = 0; }
          if (inputbuff[0] == 's' && inputbuff[1] == '=' && inputbuff[2] == 't') { mode = 1; }
          if (inputbuff[0] == 's' && inputbuff[1] == '=' && inputbuff[2] == 'a') { mode = 2; }
          if (inputbuff[0] == 's' && inputbuff[1] == '=' && inputbuff[2] == 'c') { mode = 3; }
          if (inputbuff[0] == 'a' && inputbuff[1] == '=') { sscanf((const char *)&inputbuff[2], "%d", &alt); }
          if (inputbuff[0] == 'c' && inputbuff[1] == '=') { sscanf((const char *)&inputbuff[2], "%d", &squak); }
          if (inputbuff[0] == 'r' && inputbuff[1] == '=' && inputbuff[2] == 'Y') { Annunciator = true; }
          if (inputbuff[0] == 'r' && inputbuff[1] == '=' && inputbuff[2] == 'N') { Annunciator = false; }
          if (inputbuff[0] == 'p' && inputbuff[1] == '=' && inputbuff[2] == 'Y') { hwcheck = true; }
          if (inputbuff[0] == 'p' && inputbuff[1] == '=' && inputbuff[2] == 'N') { hwcheck = false; }
        }
        inputpos = 0;
      }
    }
  }

  if (Dodisplay == true) {
    
    switch (mode) {
      case 0: sprintf(x, "OFF"); break;
      case 1: sprintf(x, "IDLE"); break;
      case 2: sprintf(x, " A "); break;
      case 3: sprintf(x, " C "); break;
      case 4: sprintf(x, "WAIT"); break;
    }

    if (ident) sprintf(buff, "Ident: ON  Mode: %s", x);
    else sprintf(buff, "Ident: OFF Mode: %s", x);
    Serial.println(buff);

    sprintf(buff, "%.5d", alt);  //pC1Characteristic->back->ID);
    Serial.print(buff);

    sprintf(buff, "%.4d", squak);  //pC1Characteristic->back->ID);
    Serial.print(buff);

    sprintf(buff, "Ann: %d", Annunciator);
    Serial.print(buff);

    sprintf(buff, "HwC: %d", hwcheck);
    Serial.print(buff);

    Dodisplay = false;
  }
  // }

  if (Debug) {
    if (Serial.available()) {
      uint8_t r = Serial.read();
      if (!confirmRequestPending) 
      {
    //    SerialBT.write(r);
        write_to_transponder(r);
      }
    }
  }
/*
  if (SerialBT.available()) 
  {
    bt_inn++;
    confirmRequestPending = false;
    timout = 0;
    
    uint8_t r = SerialBT.read();
    write_to_transponder(r);
  }
  */
}
