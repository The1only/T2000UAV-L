// Select M5core as HW and use M5Unified v0.1.16 and M5Stack v0.3.0...
#include <stdio.h>
#include "string.h"
//#include "WiFi.h"
//#include <Ticker.h>
//#include "AsyncUDP.h"
#include <Adafruit_NeoPixel.h>

//--------------------------------------------
bool Debug = false;

boolean confirmRequestPending = true;

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

int wait = 0;
int state = 0;
int m = 2;

int ser_inn = 0;
int old_ser_inn = 0;
int m_ser = 2;
int pos_ser = 0;

int timout = 0;

char altsens = 'g';

char output_buffer[50];
int output_buffer_head=0;

char input_buffer[50];
int input_buffer_head=0;
int input_buffer_tail=0;

int Power = 11;
int PIN  = 12;
#define NUMPIXELS 1

bool connected = false;

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

//--------------------------------------------
void setup() {
  pixels.begin();
  pinMode(Power,OUTPUT);
  digitalWrite(Power, HIGH);

  lastmode[0] = 't';

  Serial.begin(9600);

  if(Debug){
    while (!Serial) {
      ;  // wait for serial port to connect. Needed for native USB
    }
  }

  for(int x = 0; x < 50; x++){
    delay(100);
    if(Serial) break;
  }

  Serial.println("Transponder v1.0");
  Serial.println("9Tek AS"); 
}

//--------------------------------------------------
// This now loops and writes to my selfe...
void write_to_transponder(char r)
{
  static int ping_time = 0;

  ping_time++;
  if (r == '*' || ping_time>100)
  {
    ping_time=0;
    ping = true;
    Serial.write("*");
  }
  else
  {
    if(output_buffer_head >= sizeof(output_buffer)) output_buffer_head=0;
    output_buffer[output_buffer_head++]=r;

    if(r < 0x20){
      output_buffer[output_buffer_head]=0;

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
            input_buffer_head = sprintf(input_buffer,"a=%dM\r\n",alt);
            if(altsens == 'g') alt+=1;
          }else{
            sscanf(&output_buffer[2],"%i",&alt);                          
        //    input_buffer_head = sprintf(input_buffer,"a= %dM\r\n",alt);
          }
        break;
        
        case 'd':
          if(output_buffer[2] == 's'){
            altsens = 's';
          }else if(output_buffer[2] == 'g'){
            altsens = 'g';
          }
          if(altsens == 's') input_buffer_head = sprintf(input_buffer,"d=s\r\n");
          else if(altsens == 'g') input_buffer_head = sprintf(input_buffer,"d=g\r\n");
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

      if(input_buffer_head > 0)
      {
        Serial.write(input_buffer);
        input_buffer_head = 0;
      }
    }
  }
}

//--------------------------------------------
void send_to_transponder(char *x)
{
  int size = strlen(x);
  for(int i=0; i < size; i++) write_to_transponder(x[i]);
}

//--------------------------------------------
void loop() 
{
  static bool toggle = false;
  char x[50];
  uint8_t x2[50];
  char x3[50];

  // ----------------------
  // While we are not connected... Get some values to display in the OLED...
  if (confirmRequestPending) 
  {
    confirmRequestPending = false;
    switch (state) {
      case 0: send_to_transponder((char*)"v=1\r\n"); break;
      case 1: send_to_transponder((char*)"z=?\r\n"); break;
      case 2: send_to_transponder((char*)"c=?\r\n"); break;
      case 3: send_to_transponder((char*)"a=?\r\n"); break;
      case 4: send_to_transponder((char*)"i=?\r\n"); break;
      case 5: send_to_transponder((char*)"s=?\r\n"); break;
      case 6: send_to_transponder((char*)"r=y\r\n"); break;
//      case 7: send_to_transponder((char*)"*"); break;
    }
    if (++state > 6) state = 2;
  }

  // ----------------------
  if (send++ > 100000) 
  {
    send = 0;
    pixels.clear();

    if(toggle == false){
      toggle = true;
      digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
      pixels.setPixelColor(0, pixels.Color(0, 0, 0));
    }else
    {
      toggle = false;
      digitalWrite(LED_BUILTIN, LOW);   // turn the LED off by making the voltage LOW
      if(connected)
        pixels.setPixelColor(0, pixels.Color(0, 80, 0));
      else
        pixels.setPixelColor(0, pixels.Color(80, 0, 0));    
    }
    pixels.show();

    if(ident_timer){
      if(--ident_timer == 0){
        ident = false;
      }
    }

    // If we loose connection to bluetooth...
    if( timout++ > 20){
      timout= 20;
      connected = false;
      confirmRequestPending = true;
    }
  }

  // ----------------------
  if(Serial.available()) 
  {
    timout = 0;
    connected = true;
    uint8_t r = Serial.read();
    write_to_transponder(r);
  }
}
