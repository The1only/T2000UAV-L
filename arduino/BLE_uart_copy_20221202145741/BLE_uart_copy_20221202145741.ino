//#define T10_V20

#include <SPI.h>

//Libraries for OLED Display
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <stdio.h>
#include "string.h"
#include <TFT_eSPI.h>
#include "WiFi.h"
#include <Ticker.h>
#include "AsyncUDP.h"
#include "BluetoothSerial.h"

#if defined(T10_V20)
  #include "T10_V20.h"
  TFT_eSPI tft = TFT_eSPI();  // Invoke custom library
#endif

bool Debug = true;

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
#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

#if defined(T10_V14)
SPIClass sdSPI(VSPI);
#define IP5306_ADDR 0X75
#define IP5306_REG_SYS_CTL0 0x00
#endif

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

bool deviceConnected = false;
bool oldDeviceConnected = false;

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

//define USE_PIN // Uncomment this to use PIN during pairing. The pin is specified on the line below
#if defined(USE_PIN)
const char *pin = "1234";  // Change this to more secure PIN.
#endif

#if !defined(CONFIG_BT_SPP_ENABLED)
#error Serial Bluetooth not available or not enabled. It is only available for the ESP32 chip.
#endif

BluetoothSerial SerialBT;
boolean confirmRequestPending = true;

void BTConfirmRequestCallback(uint32_t numVal) {
  confirmRequestPending = true;
  Serial.println(numVal);
  SerialBT.confirmReply(true);
}

void BTAuthCompleteCallback(boolean success) {
  confirmRequestPending = false;
  if (success) {
    Serial.println("Pairing success!!");
  } else {
    Serial.println("Pairing failed, rejected by user!!");
  }
}

void playSound(void) {
#if defined(T10_V20)
  ledcWriteTone(CHANNEL_0, 1000);
  delay(200);
  ledcWriteTone(CHANNEL_0, 0);
#else
  digitalWrite(TFT_BL, HIGH);
  delay(200);
  digitalWrite(TFT_BL, LOW);
#endif
}

void setup() {
  pinMode(BUZZER, OUTPUT);
//  ledcSetup(CHANNEL_0, 1000, 8);
//  ledcAttachPin(BUZZER, CHANNEL_0);

  pinMode(RXD2, INPUT_PULLUP);
  pinMode(TXD2, OUTPUT);
  Serial1.begin(9600, SERIAL_8N1, RXD2, TXD2);

  Serial.begin(115200);
  Serial.println("Transponder v1.0");

  SerialBT.enableSSP();
#ifdef USE_PIN
  SerialBT.setPin(pin);
  Serial.println("Using PIN");
#endif
  SerialBT.onConfirmRequest(BTConfirmRequestCallback);
  SerialBT.onAuthComplete(BTAuthCompleteCallback);
  SerialBT.begin("Transponder");  //Bluetooth device name
  Serial.println("The device started, now you can pair it with bluetooth!");
  Serial.println("9Tek AS");

#if defined(T10_V20)
  tft.init();
  tft.setRotation(3);

  if (TFT_BL > 0) {
      pinMode(TFT_BL, OUTPUT);
      digitalWrite(TFT_BL, HIGH);
  }
  
  tft.setTextFont(1);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);

  tft.fillScreen(TFT_BLACK);
//  tft.drawString("Transponder v1.0",tft.width() / 2, tft.height() / 2);

  tft.setCursor(0, 23);
  tft.setTextSize(2);
  tft.println("   9Tek AS");
  tft.setTextSize(1);
  tft.println("    Transponder v1.0");
  delay(200);

#else
  //reset OLED display via software
  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);
  delay(20);
  digitalWrite(OLED_RST, HIGH);

  //initialize OLED
  Wire.begin(OLED_SDA, OLED_SCL);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3c, false, false)) {  // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;  // Don't proceed, loop forever
  }

  display.clearDisplay();
  display.setRotation(0);
  display.setTextColor(WHITE);
  display.setCursor(0, 23);
  display.setTextSize(2);
  display.println("  9Tek AS");
  display.setTextSize(1);
  display.println("   Transponder v1.0");
  display.display();
  delay(200);
#endif

  Serial.println("...booting...");
 // display.display();

  Serial.println("Waiting a client connection to notify...");
  delay(1000);
  playSound();
  delay(100);
  playSound();

#if defined(T10_V20)
  tft.fillScreen(TFT_BLACK);
#endif

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

void loop() {

  // While we are not connected... Get some values to display in the OLED...
  if (confirmRequestPending) {
    if (wait++ > 80000) {
      wait = 0;

      switch (state) {
        case 0:
          Serial1.println("v=1\r\n");
          break;
        case 1:
          Serial1.println("z=?\r\n");
          break;
        case 2:
          Serial1.println("c=?\r\n");
       //   SerialBT.println("c=7003\r\n");
          break;
        case 3:
          Serial1.println("a=?\r\n");
          break;
        case 4:
          Serial1.println("i=?\r\n");
          break;
        case 5:
          Serial1.println("s=?\r\n");
          break;
        case 6:
          Serial1.println("r=y\r\n");
          break;
      }
      if (++state > 6) state = 2;
    }
  }

  char x[50];
  uint8_t x2[50];
  char x3[50];

  if (send++ > 50000) {
    send = 0;
    Dodisplay = true;

    // If we loose connection to bluetooth...
    if( timout++ > 10){
      timout= 10;
      confirmRequestPending = true;
    }
  }

  if (Serial1.available()) {
    ser_inn++;
    inputbuff[inputpos] = Serial1.read();
    if (Debug) Serial.write(inputbuff[inputpos]);

    if (inputbuff[inputpos] == '*')
    {
      ping = true;
      SerialBT.println("*");
      playSound();
    }
    else{
      inputpos += 1;

      if (inputbuff[inputpos - 1] < 0x1F || inputpos >= 20) {
        if (inputpos >= 3) {
          inputbuff[inputpos - 1] = '\n';
          inputbuff[inputpos] = 0;
          if (!confirmRequestPending) {
            SerialBT.println((char *)inputbuff);
          }
//          if (Debug) Serial.println((char *)inputbuff);
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
    
#if defined(T10_V20)
//    tft.fillScreen(TFT_BLACK);
    tft.setCursor(10, 10);
    tft.setTextSize(1);

    switch (mode) {
      case 0: sprintf(x, "OFF"); break;
      case 1: sprintf(x, "IDL"); break;
      case 2: sprintf(x, " A "); break;
      case 3: sprintf(x, " C "); break;
      case 4: sprintf(x, "WAI"); break;
    }

    if (ident) sprintf(buff, "Ident: ON  Mode: %s", x);
    else sprintf(buff, "Ident: OFF Mode: %s", x);

    tft.println(buff);

    tft.setTextSize(3);
    tft.setCursor(10, 25);
    sprintf(buff, "%.5d", alt);  //pC1Characteristic->back->ID);
    tft.print(buff);

    tft.setCursor(10, 52);
    sprintf(buff, "%.4d", squak);  //pC1Characteristic->back->ID);
    tft.print(buff);

    tft.setTextSize(1);
    tft.setCursor(100, 52);
    sprintf(buff, "Ann: %d", Annunciator);

    tft.print(buff);
    tft.setCursor(100, 65);
    sprintf(buff, "HwC: %d", hwcheck);

    tft.print(buff);
    tft.setTextSize(1);

    float x = (((float)pos / 100.0) * 64.0) + 70.0;
    tft.drawFastVLine((int)x - 1, 85, 6, TFT_BLACK);
    tft.drawFastVLine((int)x, 85, 6, TFT_BLACK);
    tft.drawFastVLine((int)x + 1, 85, 6, TFT_BLACK);

    pos+=m;
    if(pos >  100) m= -2;
    if(pos < -100) m=  2;

    x = (((float)pos / 100.0) * 64.0) + 70.0;
    tft.drawFastHLine(10, 85, 128, TFT_WHITE);
    tft.drawFastVLine((int)x - 1, 85, 6, TFT_BLUE);
    tft.drawFastVLine((int)x, 85, 6, TFT_BLUE);
    tft.drawFastVLine((int)x + 1, 85, 6, TFT_BLUE);

    if(ser_inn != old_ser_inn){
      old_ser_inn = ser_inn;

      float x = (((float)pos_ser / 100.0) * 64.0) + 70.0;
      tft.drawFastVLine((int)x - 1, 95, 6, TFT_BLACK);
      tft.drawFastVLine((int)x, 95, 6, TFT_BLACK);
      tft.drawFastVLine((int)x + 1, 95, 6, TFT_BLACK);

      pos_ser+=m_ser;
      if(pos_ser >  100) m_ser= -2;
      if(pos_ser < -100) m_ser=  2;

      x = (((float)pos_ser / 100.0) * 64.0) + 70.0;
      tft.drawFastHLine(10, 95, 128, TFT_WHITE);
      tft.drawFastVLine((int)x - 1, 95, 6, TFT_BLUE);
      tft.drawFastVLine((int)x, 95, 6, TFT_BLUE);
      tft.drawFastVLine((int)x + 1, 95, 6, TFT_BLUE);
    }

    if(bt_inn != old_bt_inn){
      old_bt_inn = bt_inn;

      float x = (((float)pos_bt / 100.0) * 64.0) + 70.0;
      tft.drawFastVLine((int)x - 1, 105, 6, TFT_BLACK);
      tft.drawFastVLine((int)x, 105, 6, TFT_BLACK);
      tft.drawFastVLine((int)x + 1, 105, 6, TFT_BLACK);

      pos_bt+=m_bt;
      if(pos_bt >  100) m_bt= -2;
      if(pos_bt < -100) m_bt=  2;

      x = (((float)pos_bt / 100.0) * 64.0) + 70.0;
      tft.drawFastHLine(10, 105, 128, TFT_WHITE);
      tft.drawFastVLine((int)x - 1, 105, 6, TFT_BLUE);
      tft.drawFastVLine((int)x, 105, 6, TFT_BLUE);
      tft.drawFastVLine((int)x + 1, 105, 6, TFT_BLUE);
    }

#else
    display.clearDisplay();
    display.setCursor(5, 5);
    display.setTextSize(1);

    switch (mode) {
      case 0: sprintf(x, "OFF"); break;
      case 1: sprintf(x, "IDL"); break;
      case 2: sprintf(x, " A "); break;
      case 3: sprintf(x, " C "); break;
      case 4: sprintf(x, "WAI"); break;
    }

    if (ident) sprintf(buff, "Ident: ON  Mode: %s", x);
    else sprintf(buff, "Ident: OFF Mode: %s", x);

    display.println(buff);

    display.setTextSize(3);
    display.setCursor(5, 15);
    sprintf(buff, "%.5d", alt);  //pC1Characteristic->back->ID);
    display.print(buff);

    display.setCursor(5, 40);
    sprintf(buff, "%.4d", squak);  //pC1Characteristic->back->ID);
    display.print(buff);

    display.setTextSize(1);
    display.setCursor(90, 42);
    sprintf(buff, "Ann: %d", Annunciator);

    display.print(buff);
    display.setCursor(90, 53);
    sprintf(buff, "HwC: %d", hwcheck);

    display.print(buff);
    display.setTextSize(1);
/*
    float x = (((float)pos / 100.0) * 64.0) + 64.0;
    tft.drawFastVLine((int)x - 1, 56, 6, TFT_BLACK);
    display.drawFastVLine((int)x, 56, 6, TFT_BLACK);
    tft.drawFastVLine((int)x + 1, 56, 6, TFT_BLACK);
*/
    pos+=m;
    if(pos >  100) m= -2;
    if(pos < -100) m=  2;
    
    float x = (((float)pos / 100.0) * 64.0) + 64.0;
    display.drawFastHLine(0, 63, 128, WHITE);
    display.drawFastVLine((int)x - 1, 56, 6, WHITE);
    display.drawFastVLine((int)x, 56, 6, WHITE);
    display.drawFastVLine((int)x + 1, 56, 6, WHITE);

    // display.drawFastVLine(64, 55, 10, WHITE);

    if(ser_inn != old_ser_inn){
      old_ser_inn = ser_inn;

      float x = (((float)pos_ser / 100.0) * 64.0) + 70.0;
      display.drawFastVLine((int)x - 1, 95, 6, TFT_BLACK);
      display.drawFastVLine((int)x, 95, 6, TFT_BLACK);
      display.drawFastVLine((int)x + 1, 95, 6, TFT_BLACK);

      pos_ser+=m_ser;
      if(pos_ser >  100) m_ser= -2;
      if(pos_ser < -100) m_ser=  2;

      x = (((float)pos_ser / 100.0) * 64.0) + 70.0;
      display.drawFastHLine(10, 95, 128, TFT_WHITE);
      display.drawFastVLine((int)x - 1, 95, 6, TFT_BLUE);
      display.drawFastVLine((int)x, 95, 6, TFT_BLUE);
      display.drawFastVLine((int)x + 1, 95, 6, TFT_BLUE);
    }

    if(bt_inn != old_bt_inn){
      old_bt_inn = bt_inn;

      float x = (((float)pos_bt / 100.0) * 64.0) + 70.0;
      display.drawFastVLine((int)x - 1, 105, 6, TFT_BLACK);
      display.drawFastVLine((int)x, 105, 6, TFT_BLACK);
      display.drawFastVLine((int)x + 1, 105, 6, TFT_BLACK);

      pos_bt+=m_bt;
      if(pos_bt >  100) m_bt= -2;
      if(pos_bt < -100) m_bt=  2;

      x = (((float)pos_bt / 100.0) * 64.0) + 70.0;
      display.drawFastHLine(10, 105, 128, TFT_WHITE);
      display.drawFastVLine((int)x - 1, 105, 6, TFT_BLUE);
      display.drawFastVLine((int)x, 105, 6, TFT_BLUE);
      display.drawFastVLine((int)x + 1, 105, 6, TFT_BLUE);
    }

    display.display();
#endif

    Dodisplay = false;
  }
  // }

  if (Debug) {
    if (Serial.available()) {
      uint8_t r = Serial.read();
      if (!confirmRequestPending) {
        SerialBT.write(r);
        Serial1.write(r);
      }
    }
  }

  if (SerialBT.available()) {
    bt_inn++;
    confirmRequestPending = false;
    timout = 0;
    
    uint8_t r = SerialBT.read();
    Serial1.write(r);
    if (Debug) Serial.write(r);
  }
}
