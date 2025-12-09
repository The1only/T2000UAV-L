/*
NRA24 millimeter wave radar

NRA24 is compact K-band radar altimeter developed by Hunan Nanoradar Science and
Technology Co., Ltd. It adopts 24GHz-ISM frequency band, with the advantages of 2cm
measuring accuracy, small size, high sensitivity, light weight, easy integration and stable
performance, which satisfies the application requirements in unmanned aircraft system (UAS),
helicopters, small airships and other field.

AA AA 0C 07 01 B8 05 4E 2D 02 BE F9 55 55  <- I receive...
AA AA 0C 07 01 D8 05 4E 2D 02 BE 19 55 55  <- I receive...
AA AA 0C 07 01 44 05 4E 2D 02 BE 85 55 55  <- I receive...
AA AA 0C 07 01 70 05 4E 2D 02 BE B1 55 55  <- I receive...
AA AA 0C 07 01 8C 05 4E 2D 02 BE CD 55 55  <- I receive...
AA AA 0C 07 01 CC 05 4E 2D 02 BE 0D 55 55  <- I receive...
AA AA 0C 07 01 30 05 4E 2D 02 BE 71 55 55  <- I receive...

*/

// Must be before any includes
#undef USB_MANUFACTURER
#undef USB_PRODUCT

#define USB_MANUFACTURER "9Tek"
#define USB_PRODUCT      "Radar_v1"
#define USB_SERIAL       "Radar001"   // optional

#undef DEBUG
#define SIMULATE

String inputString = "";  // a String to hold incoming data
String outputString = "";
bool stringComplete = false;  // whether the string is complete

uint8_t command[] = {
  0xAA, 0xAA,
  0x00, 0x02,
  0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00,
  0x55, 0x55 
};

uint8_t commandStatus[] = {
  0xAA, 0xAA,                                // Header
  0x00, 0x02,                                // Command
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Payload
  0x00,                                      // Checksum or reserved
  0x55, 0x55                                 // Footer
};

byte myData[] = {0xAA, 0xAA, 0x0C, 0x07,   0x01, 0xA0, 0x1, 0x01, 0x01, 0x01, 0x01, 0x71,   0x55, 0x55};

void setupRADAR() 
{
  // reserve 200 bytes for the inputString:
  inputString.reserve(200);
  sendCommand(command);

}

void calculatePayloadChecksum(uint8_t *data) {
  uint16_t sum = 0;
  for (int i = 4; i < 4 + 7; i++) {
    sum += data[i];
  }
  data[11] = (uint8_t)(sum & 0xFF);
}

void sendCommand(uint8_t *cmd) {
  
  char buff[10];
//  calculatePayloadChecksum(cmd);
  for (int i =0; i < 14; i++) {
    sprintf(buff, "%.2X ", cmd[i]);
    Serial.print(buff);
  }
  Serial.write(" <- I send...\n");

  Serial1.write(cmd, sizeof(command));
}

void loopRADAR() 
{  
  static int pos = 0;
  int i=0;
  char buff[50];
  static bool l_toggle=true;
  static unsigned int l_sleep = 0;
  static bool connected = false;

#ifdef SIMULATE
  delay(50);

  myData[4+3]+=16;
  if(myData[4+3] == 1) myData[4+2]++; 
  if(myData[4+2] > 40) myData[4+2]=0;

  myData[4+6]+=4;
  if(myData[4+6] == 1) myData[4+5]++; 
  if(myData[4+5] > 10) myData[4+5]=0;

  outputString= String((const char*)myData);
  stringComplete = true;
#endif

//-----------------------------------------
  // print the string when a newline arrives:
  if (stringComplete) {
    if(outputString[0] == 0xAA && outputString[1] == 0xAA && outputString[2] == 0x0C && outputString[3] == 0x07)
    {
      connected = true;
      char index = outputString[4+0];
      char tmp = outputString[4+1];
      float section = (((float)tmp*0.5)-50.0)/100.0;
//      float section = ((float)outputString[4+1]*0.5)-50.0;
      float VrelH   = ((float)outputString[4+2]*256.0 + (float)outputString[4+3])*0.006;
      float range    =((float)outputString[4+5]*256.0 + (float)outputString[4+6])*0.05; //-35;

      sprintf(buff, "%f,%f,%f\n",section,VrelH,range);
      for (int i = 0; i < MAX_SRV_CLIENTS; i++)
        if (serverClients[i] && serverClients[i].connected())
          serverClients[i].write(buff, strlen(buff));
    }
    // clear the string:
    outputString = "";
    stringComplete = false;
  }

  while (Serial1.available()) {
    char inChar = (char)Serial1.read();
    inputString += inChar;

    if (pos == 0 && inChar == 0x55) {
      pos = 1;
    } else if (pos == 1 && inChar == 0x55) {
        pos = 0;
        stringComplete = true;
        outputString = inputString;
        inputString = "";
    } else{
        pos = 0;
    }
    
  }
}
