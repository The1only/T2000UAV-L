/*
 * ESP32 Telnet bridge + multicast announcer
 */

#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiUdp.h>

WiFiMulti wifiMulti;

// how many clients should be able to telnet to this ESP32
#define use_serial false
#define MAX_SRV_CLIENTS 1

// Alternative accespoints...
const char *ssid     = "Hvattum";
const char *password = "Jordvarme@2023@";

const char *ssid1    = "Aeros2";
const char *password1= "Terjenilsen1";

const char *ssid2    = "9Tek_printer";
const char *password2 = "didiinne";

// telnet server
WiFiServer server(23);
WiFiClient serverClients[MAX_SRV_CLIENTS];

// multicast
WiFiUDP udp;
const IPAddress MULTICAST_ADDR(239, 255, 0, 1);
const uint16_t MULTICAST_PORT = 4210;
unsigned long lastAnnounce = 0;
const unsigned long ANNOUNCE_INTERVAL_MS = 5000; // every 5 seconds

void setup() {
  Serial.begin(9600);
  Serial.println();
  Serial.println("Connecting");

  // Add multiple APs
  wifiMulti.addAP(ssid, password);
  wifiMulti.addAP(ssid1, password1);
  wifiMulti.addAP(ssid2, password2);

  Serial.println("Connecting WiFi...");
  for (int loops = 10; loops > 0; loops--) {
    if (wifiMulti.run() == WL_CONNECTED) {
      Serial.println();
      Serial.print("WiFi connected. IP address: ");
      Serial.println(WiFi.localIP());
      break;
    } else {
      Serial.print("Retry in ");
      Serial.print(loops);
      Serial.println(" s");
      delay(100);
    }
  }

  if (wifiMulti.run() != WL_CONNECTED) {
    Serial.println("WiFi connect failed, restarting...");
    delay(1000);
    ESP.restart();
  }

  // Start UDP multicast (ESP32 core 3.x: 2-arg version)
  if (udp.beginMulticast(MULTICAST_ADDR, MULTICAST_PORT)) {
    Serial.print("Multicast started on ");
    Serial.print(MULTICAST_ADDR);
    Serial.print(':');
    Serial.println(MULTICAST_PORT);

    // Send one initial multicast announcement
    udp.beginPacket(MULTICAST_ADDR, MULTICAST_PORT);
    udp.print("ESP32 online, IP=");
    udp.print(WiFi.localIP());
    udp.endPacket();
  } else {
    Serial.println("Failed to start multicast UDP");
  }

  // Start UART (if enabled) and telnet server
  if (use_serial) {
    Serial1.begin(9600);
  }
  server.begin();
  server.setNoDelay(true);

  Serial.print("Ready! Use 'telnet ");
  Serial.print(WiFi.localIP());
  Serial.println(" 23' to connect");
}

void loop() {
  uint8_t i;

  if (wifiMulti.run() == WL_CONNECTED) {

    // Periodic multicast announce with IP
    unsigned long now = millis();
    if (now - lastAnnounce >= ANNOUNCE_INTERVAL_MS) {
      lastAnnounce = now;

      udp.beginPacket(MULTICAST_ADDR, MULTICAST_PORT);
      udp.print("WTGAHRS1 ");
      udp.print(WiFi.localIP());
      udp.endPacket();
    
    //  Serial.println();
    //  Serial.print("WiFi Multicast. IP address: ");
      //Serial.println(WiFi.localIP());
     // Serial.println(MULTICAST_PORT);
    }

    // check if there are any new clients
    if (server.hasClient()) {
      for (i = 0; i < MAX_SRV_CLIENTS; i++) {
        // find free/disconnected spot
        if (!serverClients[i] || !serverClients[i].connected()) {
          if (serverClients[i]) {
            serverClients[i].stop();
          }
          serverClients[i] = server.available();
          if (!serverClients[i]) {
            Serial.println("available broken");
          }
          Serial.print("New client: ");
          Serial.print(i);
          Serial.print(' ');
          Serial.println(serverClients[i].remoteIP());
          break;
        }
      }
      if (i >= MAX_SRV_CLIENTS) {
        // no free/disconnected spot so reject
        WiFiClient tmp = server.available();
        if (tmp) {
          tmp.stop();
        }
      }
    }

    // check clients for data
    for (i = 0; i < MAX_SRV_CLIENTS; i++) {
      if (serverClients[i] && serverClients[i].connected()) {
        if (serverClients[i].available()) {
          // get data from the telnet client and push it to the UART or Serial
          while (serverClients[i].available()) {
            int ch = serverClients[i].read();
            if (use_serial) {
              Serial1.write(ch);
            } else {
              Serial.write(ch);
            }
          }
        }
      } else {
        if (serverClients[i]) {
          serverClients[i].stop();
        }
      }
    }

    // check UART/Serial for data and forward to telnet clients
    if (use_serial) {
      if (Serial1.available()) {
        size_t len = Serial1.available();
        uint8_t sbuf[len];
        Serial1.readBytes(sbuf, len);
        Serial1.write((char *)sbuf); // local echo (optional)
        // push UART data to all connected telnet clients
        for (i = 0; i < MAX_SRV_CLIENTS; i++) {
          if (serverClients[i] && serverClients[i].connected()) {
            serverClients[i].write(sbuf, len);
          }
        }
      }
    } else {
      if (Serial.available()) {
        size_t len = Serial.available();
        uint8_t sbuf[len];
        Serial.readBytes(sbuf, len);
        // push Serial data to all connected telnet clients
        for (i = 0; i < MAX_SRV_CLIENTS; i++) {
          if (serverClients[i] && serverClients[i].connected()) {
            serverClients[i].write(sbuf, len);
          }
        }
      }
    }

  } else {
    Serial.println("WiFi not connected!");
    for (i = 0; i < MAX_SRV_CLIENTS; i++) {
      if (serverClients[i]) {
        serverClients[i].stop();
      }
    }
    delay(1000);
  }
}