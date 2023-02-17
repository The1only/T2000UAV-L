//Libraries for LoRa
#include <SPI.h>
#include <LoRa.h>

//Libraries for OLED Display
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <stdio.h>
#include "string.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_attr.h"
#include "soc/rtc.h"
#include "driver/mcpwm.h"
#include <soc/mcpwm_struct.h>

#include "WiFi.h"
#include <Ticker.h>
#include "AsyncUDP.h"
#include "BluetoothSerial.h"

bool Debug = false;

//define the pins used by the LoRa transceiver module
#define OLED_SDA 4
#define SCK 5
#define RXD2 17
#define TXD2 12
#define RST 14
#define OLED_SCL 15
#define OLED_RST 16
#define SS 18
#define MISO 19
#define BUZZER 27
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
int mode = 0;
int alt = 0;
int squak = 7000;
int pos = 30;

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

void setup() {
  pinMode(BUZZER, OUTPUT);
  pinMode(RXD2, INPUT);
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
  SerialBT.begin("ESP32test");  //Bluetooth device name
  Serial.println("The device started, now you can pair it with bluetooth!");

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
  display.setTextColor(WHITE);
  display.setCursor(0, 23);
  display.setTextSize(2);
  display.println("  9Tek AS");
  Serial.println("9Tek AS");
  display.setTextSize(1);
  display.println("   Transponder v1.0");
  display.display();
  delay(200);

  Serial.println("...booting...");
  display.display();

  Serial.println("Waiting a client connection to notify...");
  delay(2000);

  Serial1.println("c=?\r\n");
  delay(100);
  Serial1.println("a=?\r\n");
}

//--------------------------------------------------
int wait = 0;
int state = 0;
void loop() {

  // While we are not connected... Get some values to display in the OLED...
  if (confirmRequestPending) {
    if (wait++ > 10000) {
      wait = 0;
      if(Debug) Serial.println(".");

      switch(state){
        case 0:
          Serial1.println("c=?\r\n");
        break;
        case 1:
          Serial1.println("a=?\r\n");
        break;
        case 2:
          Serial1.println("s=?\r\n");
        break;
      }
      if(++state > 2) state = 0;
    }
  }

  char x[50];
  uint8_t x2[50];
  char x3[50];

  if (send++ > 500) {
    send = 0;
    Dodisplay = true;
  }

  if (Serial1.available()) {
    inputbuff[inputpos] = Serial1.read();
    inputpos += 1;

    if (inputbuff[inputpos - 1] < 0x1F || inputpos >= 20) {
      if (inputpos >= 3) {
        inputbuff[inputpos - 1] = '\n';
        inputbuff[inputpos] = 0;
        if (!confirmRequestPending) {
          SerialBT.println((char *)inputbuff);
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
      }
      inputpos = 0;
    }
  }

  if (Dodisplay == true) {
    display.clearDisplay();
    display.setCursor(5, 5);
    display.setTextSize(1);

    switch (mode) {
      case 0: sprintf(x, "OFF"); break;
      case 1: sprintf(x, "IDL"); break;
      case 2: sprintf(x, " A "); break;
      case 3: sprintf(x, " C "); break;
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
    //      sprintf(buff, "%.5d", pC1Characteristic->back->ID);
    display.print(buff);
    display.setCursor(90, 53);
    //      sprintf(buff, "%.5d", pC2Characteristic->back->ID);
    display.print(buff);

    display.setTextSize(1);

    float x = (((float)pos / 100.0) * 64.0) + 64.0;
    display.drawFastHLine(0, 63, 128, WHITE);
    display.drawFastVLine((int)x - 1, 56, 6, WHITE);
    display.drawFastVLine((int)x, 56, 6, WHITE);
    display.drawFastVLine((int)x + 1, 56, 6, WHITE);

    // display.drawFastVLine(64, 55, 10, WHITE);

    display.display();
    Dodisplay = false;
  }
  // }

  if(Debug){ 
    if (Serial.available()) {
      uint8_t r = Serial.read();
      if (!confirmRequestPending) {
        SerialBT.write(r);
      }
    }
  }

  if (SerialBT.available()) {
    confirmRequestPending = false;
    uint8_t r = SerialBT.read();
    Serial1.write(r);
    if (Debug) Serial.write(r);
  }
}
