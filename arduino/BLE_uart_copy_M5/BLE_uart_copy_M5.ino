// Select M5core as HW and use M5Unified v0.1.16 and M5Stack v0.3.0...
#define simulate

#include "M5Unified.h"
#include "BluetoothSerial.h"
#define tft M5.Lcd

#include <stdio.h>
#include "string.h"
//#include "WiFi.h"
//#include <Ticker.h>
//#include "AsyncUDP.h"
//--------------------------------------------

#define NOTE_DH1 589
#define NOTE_DH2 661
#define NOTE_DH3 700
#define NOTE_DH4 786
#define NOTE_DH5 882
#define NOTE_DH6 990
#define NOTE_DH7 112

#define RXD2 17
#define TXD2 12

//433E6 for Asia
//866E6 for Europe
//915E6 for North America
#define BAND 866E6

//define USE_PIN // Uncomment this to use PIN during pairing. The pin is specified on the line below
#if defined(USE_PIN)
const char *pin = "1234";  // Change this to more secure PIN.
#endif

//--------------------------------------------
bool Debug = true;

BluetoothSerial SerialBT;
boolean confirmRequestPending = true;

bool Dodisplay = false;
long send = 0;

char buff[100];
bool ident = 0;
int  ident_timer = 0;
int  mode = 1;
char lastmode[2] = {'t',0};
int  alt = 0;
int  squak = 7000;
int  pos = 30;
bool Annunciator = false;
bool ping = false;
bool hwcheck = false;

uint8_t inputbuff[100];
int inputpos = 0;

bool deviceConnected = false;
bool oldDeviceConnected = false;

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

//--------------------------------------------
void BTConfirmRequestCallback(uint32_t numVal) {
  confirmRequestPending = true;
  Serial.println(numVal);
  SerialBT.confirmReply(true);
}

//--------------------------------------------
void BTAuthCompleteCallback(boolean success) {
  confirmRequestPending = false;
  if (success) {
    Serial.println("Pairing success!!");
  } else {
    Serial.println("Pairing failed, rejected by user!!");
  }
}

//--------------------------------------------
void playSound(int tone) {    
  M5.Speaker.setVolume(255);
  M5.Speaker.tone(tone, 500); //frequency 3000, with a duration of 2000ms    
  delay(750);

}

//--------------------------------------------
void setup() {
  M5.begin();
  M5.Power.begin();

  lastmode[0] = 't';
    
  playSound(NOTE_DH6);

  tft.setRotation(1);

#ifndef simulate
  pinMode(RXD2, INPUT_PULLUP);
  pinMode(TXD2, OUTPUT);
  Serial1.begin(9600, SERIAL_8N1, RXD2, TXD2);
#endif

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
 
  tft.setTextFont(1);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  M5.Lcd.fillScreen(TFT_BLACK);
  M5.Lcd.setCursor(0, 25);
  M5.Lcd.setTextSize(4);
  M5.Lcd.println("   9Tek AS");
  M5.Lcd.setTextSize(2);
  M5.Lcd.println("    Transponder v1.0");
  delay(2000);

  M5.Lcd.fillScreen(TFT_BLACK);
  Serial.println("...booting...");
  Serial.println("Waiting a client connection to notify...");  
}

//--------------------------------------------------
void write_to_transponder(char r)
{
#ifndef simulate
  if (Debug) Serial.write(r);
  Serial1.write(r);
#else
  if(output_buffer_head >= sizeof(output_buffer)) output_buffer_head=0;
  output_buffer[output_buffer_head++]=r;

  if(r < 0x20){
    output_buffer[output_buffer_head]=0;
    alt+=1;

    switch(output_buffer[0]){
      case 'z':
        input_buffer_head = sprintf(input_buffer,"T2000U xx-x.x.x\r\n");
      break;

      case 'c':
        if(output_buffer[2] == '?'){
          input_buffer_head = sprintf(input_buffer,"c=%d\r\n",squak);
        }else{
          sscanf(&output_buffer[2],"%i",&squak);     
          input_buffer_head = sprintf(input_buffer,"c=%d\r\n",squak);
        }
      break;

      case 'a':
        if(output_buffer[2] == '?'){
          input_buffer_head = sprintf(input_buffer,"a= %dM\r\n",alt);
        }else{
          sscanf(&output_buffer[2],"%i",&alt);                          
          input_buffer_head = sprintf(input_buffer,"a= %dM\r\n",alt);
        }
      break;

      case 'i':
        if(output_buffer[2] == '?'){
          input_buffer_head = sprintf(input_buffer,"i=%s\r\n",(ident==true)?"1":"0");
        }else{
          if(output_buffer[2] == 's'){
            ident=true;
            ident_timer=18;
            input_buffer_head = sprintf(input_buffer,"i=%s\r\n",(ident==true)?"1":"0");
          }                         
        }
      break;

      case 's':
        if(output_buffer[2] != '?') lastmode[0] = output_buffer[2];  
        input_buffer_head = sprintf(input_buffer,"s=%s\r\n",lastmode);
      break;

      case 'r':
        input_buffer_head = sprintf(input_buffer,"r=%s\r\n",(Annunciator==true)?"Y":"N"); // ramdomize...
      break;

      case 'p':
        input_buffer_head = sprintf(input_buffer,"p=%s\r\n",(hwcheck==true)?"Y":"N"); // RANDOMIZE....
      break;

      default:
        input_buffer_head = 0;
      break;
    }
    input_buffer_tail = 0;
    output_buffer_head=0;
  }
#endif
}

//--------------------------------------------
void send_to_transponder(char *x)
{
  int size = strlen(x);
  for(int i=0; i < size; i++) write_to_transponder(*x);
}

//--------------------------------------------
void loop() 
{
  M5.update(); 
  
//  if (M5.BtnA.wasPressed()) { 
//  }

  // While we are not connected... Get some values to display in the OLED...
  if (confirmRequestPending) 
  {
    if (wait++ > 80000) {
      wait = 0;
      switch (state) {
        case 0: send_to_transponder("v=1\r\n"); break;
        case 1: send_to_transponder("z=?\r\n"); break;
        case 2: send_to_transponder("c=?\r\n"); break;
        case 3: send_to_transponder("a=?\r\n"); break;
        case 4: send_to_transponder("i=?\r\n"); break;
        case 5: send_to_transponder("s=?\r\n"); break;
        case 6: send_to_transponder("r=y\r\n"); break;
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

    if(ident_timer){
      if(--ident_timer == 0){
        ident = false;
      }
    }

    // If we loose connection to bluetooth...
    if( timout++ > 10){
      timout= 10;
      confirmRequestPending = true;
    }
  }

#ifdef simulate
  while(input_buffer_tail != input_buffer_head) {
    inputbuff[inputpos] = input_buffer[input_buffer_tail];
//    if(inputbuff[inputpos] < 0x20){
//      input_buffer_tail = 0;
//      input_buffer_head = 0;
//    }
//    else{
      if(++input_buffer_tail >= sizeof(input_buffer)) input_buffer_tail=0;
//    }
#else
  if (Serial1.available()) {
    inputbuff[inputpos] = Serial1.read();
#endif
    ser_inn++;

    if (inputbuff[inputpos] == '*')
    {
      ping = true;
      SerialBT.println("*");
    }
    else{
      inputpos += 1;

      if (inputbuff[inputpos - 1] < 0x1F || inputpos >= 20) {
        if (inputpos >= 3) {
          inputbuff[inputpos - 1] = '\n';
          inputbuff[inputpos] = 0;

          if (Debug) Serial.write((char *)inputbuff);

          if (!confirmRequestPending) 
          {
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

  // ----------------------
  // ----------------------
  if (Dodisplay == true) 
  {    
//    M5.Lcd.fillScreen(TFT_BLACK);
    M5.Lcd.setCursor(10, 10);
    M5.Lcd.setTextSize(2);

    switch (mode) {
      case 0: sprintf(x, "OFF "); break;
      case 1: sprintf(x, "STBY"); break;
      case 2: sprintf(x, "NORM"); break;
      case 3: sprintf(x, "ALT "); break;
    }
    if (ident) sprintf(buff, "Ident: ON    Mode: %s", x);
    else sprintf(      buff, "Ident: OFF   Mode: %s", x);
    M5.Lcd.println(buff);

    M5.Lcd.setTextSize(3);
    M5.Lcd.setCursor(10, 45);
    sprintf(buff, "Altitude: %.5d", alt);  //pC1Characteristic->back->ID);
    M5.Lcd.print(buff);

    M5.Lcd.setCursor(10, 92);
    sprintf(buff, "Squak:    %.4d", squak);  //pC1Characteristic->back->ID);
    M5.Lcd.print(buff);

    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(10, 135);
    sprintf(buff, "Ann: %d", Annunciator);

    M5.Lcd.print(buff);
    M5.Lcd.setCursor(175, 135);
    sprintf(buff, "HwC: %d", hwcheck);

    M5.Lcd.print(buff);
    M5.Lcd.setTextSize(1);

    // ----------------------
    float x = ((float)pos / 100.0) * 320;
    M5.Lcd.drawFastVLine((int)x - 1, 155, 15, TFT_BLACK);
    M5.Lcd.drawFastVLine((int)x, 155, 15, TFT_BLACK);
    M5.Lcd.drawFastVLine((int)x + 1, 155, 15, TFT_BLACK);

    pos+=m;
    if(pos >  100) m= -4;
    if(pos <= 0)   m=  4;

    x = ((float)pos / 100.0) * 320;
    M5.Lcd.drawFastVLine((int)x - 1, 155, 15, TFT_RED);
    M5.Lcd.drawFastVLine((int)x, 155, 15, TFT_RED);
    M5.Lcd.drawFastVLine((int)x + 1, 155, 15, TFT_RED);
    M5.Lcd.drawFastHLine(10, 155, 320, TFT_WHITE);

    // ----------------------
    if(ser_inn != old_ser_inn){
      old_ser_inn = ser_inn;

      float x = ((float)pos_ser / 100.0) * 320;
      M5.Lcd.drawFastVLine((int)x - 1, 185, 15, TFT_BLACK);
      M5.Lcd.drawFastVLine((int)x, 185, 15, TFT_BLACK);
      M5.Lcd.drawFastVLine((int)x + 1, 185, 15, TFT_BLACK);

      pos_ser+=m_ser;
      if(pos_ser >  100) m_ser= -4;
      if(pos_ser <= 0)   m_ser=  4;

      x = ((float)pos_ser / 100.0) * 320;
      M5.Lcd.drawFastVLine((int)x - 1, 185, 15, TFT_RED);
      M5.Lcd.drawFastVLine((int)x, 185, 15, TFT_RED);
      M5.Lcd.drawFastVLine((int)x + 1, 185, 15, TFT_RED);
    }
    M5.Lcd.drawFastHLine(10, 185, 320, TFT_WHITE);

    // ----------------------
    if(bt_inn != old_bt_inn){
      old_bt_inn = bt_inn;

      float x = ((float)pos_bt / 100.0) * 320;
      M5.Lcd.drawFastVLine((int)x - 1, 210, 15, TFT_BLACK);
      M5.Lcd.drawFastVLine((int)x, 210, 15, TFT_BLACK);
      M5.Lcd.drawFastVLine((int)x + 1, 210, 15, TFT_BLACK);

      pos_bt+=m_bt;
      if(pos_bt >  100) m_bt= -4;
      if(pos_bt <= 0)   m_bt=  4;

      x = ((float)pos_bt / 100.0) * 320;
      M5.Lcd.drawFastVLine((int)x - 1, 210, 15, TFT_RED);
      M5.Lcd.drawFastVLine((int)x, 210, 15, TFT_RED);
      M5.Lcd.drawFastVLine((int)x + 1, 210, 15, TFT_RED);
    }
    M5.Lcd.drawFastHLine(10, 210, 320, TFT_WHITE);

    // ----------------------
    Dodisplay = false;
  }
  // }

  if (Debug) {
    if (Serial.available()) {
      uint8_t r = Serial.read();
      if (!confirmRequestPending) 
      {
  //      SerialBT.write(r);
        write_to_transponder(r);
      }
    }
  }

  if (SerialBT.available()) 
  {
    bt_inn++;
    confirmRequestPending = false;
    timout = 0;
    
    uint8_t r = SerialBT.read();
    write_to_transponder(r);
  }
}
