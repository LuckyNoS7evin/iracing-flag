/*
 This example connects to an unencrypted WiFi network.
 Then it prints the MAC address of the WiFi module,
 the IP address obtained, and other network details.

 created 13 July 2010
 by dlf (Metodo2 srl)
 modified 31 May 2012
 by Tom Igoe
 */
#include <SPI.h>
#include <WiFiNINA.h>
#include <WiFiUdp.h>
#include <Adafruit_NeoPixel.h>

#include "arduino_secrets.h" 
///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
int status = WL_IDLE_STATUS;     // the WiFi radio's status

int RELAY_PIN = 8;
int LED_PIN = 6;
int NUM_LEDS = 300;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

unsigned int localPort = 2390;      // local port to listen on

char packetBuffer[256]; //buffer to hold incoming packet
char  ReplyBuffer[] = "acknowledged";       // a string to send back

bool runningLightScript = false;
bool blueFlag = false;

WiFiUDP Udp;

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);
  delay(500);
  strip.begin();
  strip.show();

  strip.setPixelColor(0, strip.Color(255, 0, 0));
  strip.show();
  delay(500);

  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }

  // you're connected now, so print out the data:
  Serial.print("You're connected to the network");
  printCurrentNet();
  printWifiData();
  Udp.begin(localPort);
  strip.setPixelColor(0, strip.Color(0, 255, 0));
  strip.show();
  delay(500);
  digitalWrite(RELAY_PIN, LOW);
  runYellowBlue();
}

void loop() {
  // if there's data available, read a packet
  int packetSize = Udp.parsePacket();
  if (packetSize) {
    Serial.print("Received packet of size ");
    Serial.println(packetSize);
    Serial.print("From ");
    IPAddress remoteIp = Udp.remoteIP();
    Serial.print(remoteIp);
    Serial.print(", port ");
    Serial.println(Udp.remotePort());

    // read the packet into packetBufffer
    int len = Udp.read(packetBuffer, 255);
    if (len > 0) {
      packetBuffer[len] = 0;
    }
    Serial.println("Contents:");
    Serial.println(packetBuffer);
    runFlagScript(String(packetBuffer));

    // send a reply, to the IP address and port that sent us the packet we received
    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    Udp.write(ReplyBuffer);
    Udp.endPacket();
  }
  runBlueFlag();
}

void RGBLoop(){
  for(int j = 0; j < 3; j++ ) {
    // Fade IN
    for(int k = 0; k < 256; k++) {
      switch(j) {
        case 0: setAll(k,0,0); break;
        case 1: setAll(0,k,0); break;
        case 2: setAll(0,0,k); break;
      }
      showStrip();
      //delay(3);
    }
    // Fade OUT
    for(int k = 255; k >= 0; k--) {
      switch(j) {
        case 0: setAll(k,0,0); break;
        case 1: setAll(0,k,0); break;
        case 2: setAll(0,0,k); break;
      }
      showStrip();
      //delay(3);
    }
  }
}

void runFlagScript(String packetBuffer) {
  if (!runningLightScript) {
    runningLightScript = true;
    packetBuffer.trim();
    Serial.println("\nHere: " + packetBuffer + ":::");
    if (packetBuffer == "clear") {
      digitalWrite(RELAY_PIN, HIGH);
      setAll(0,0,0);
      showStrip();
      delay(250);
      digitalWrite(RELAY_PIN, LOW);
    }
    if (packetBuffer == "irsdk_startReady") {
      digitalWrite(RELAY_PIN, HIGH);
      delay(250);
      setAll(0,0,0);
      for (int i = 0; i <= strip.numPixels(); i += 3) {
        setPixel(i, 255, 0, 0);
      }
      showStrip();
    }
    if (packetBuffer == "irsdk_startSet") {
      digitalWrite(RELAY_PIN, HIGH);
      delay(250);
      setAll(255, 0, 0);
      showStrip();
    }
    if (packetBuffer == "irsdk_startGo") {
      digitalWrite(RELAY_PIN, HIGH);
      delay(250);
      for (int i = 0; i < 8; i++) {
        setAll(0,0,0);
        if ( i % 2 == 0) {
          setAll(0, 255, 0);
        }
        showStrip();
        delay(250);
      }
      setAll(0, 255, 0);
      showStrip();
      delay(2000);
      setAll(0,0,0);
      showStrip();
      delay(250);
      digitalWrite(RELAY_PIN, LOW);
    }
    if (packetBuffer == "irsdk_blue") {
      digitalWrite(RELAY_PIN, HIGH);
      blueFlag = true;
    }
    if (packetBuffer == "clear_blue") {
      blueFlag = false;
      digitalWrite(RELAY_PIN, LOW);
    }
    if (packetBuffer == "irsdk_oneLapToGreen") {
      digitalWrite(RELAY_PIN, HIGH);
    
    }
    if (packetBuffer == "irsdk_checkered") {
      digitalWrite(RELAY_PIN, HIGH);
      delay(250);
      for (int i = 0; i < 8; i++) {
        setAll(0, 0, 0);
        for (int j = 0; j < strip.numPixels(); j++) {
          if(j % 8 == 0) {
            if ( i % 2 == 0) {
                setPixel(j, 255,255,255); 
                setPixel(j+1, 255,255,255); 
                setPixel(j+2, 255,255,255); 
                setPixel(j+3, 255,255,255); 
              
            } else {
                setPixel(j+4, 255,255,255); 
                setPixel(j+5, 255,255,255); 
                setPixel(j+6, 255,255,255); 
                setPixel(j+7, 255,255,255); 
            }
          }
        }
        showStrip();
        delay(500);
      }
      delay(2000);
      setAll(0,0,0);
      showStrip();
      delay(250);
      digitalWrite(RELAY_PIN, LOW);
    }
    runningLightScript = false;
  }
}

void runBlueFlag() {
  if(blueFlag){
    colorChaseFour(strip.Color(0,0,255), 25);
  }
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
      strip.show();
      delay(wait);
  }
}

void colorChaseFour(uint32_t c, uint8_t wait) {
  uint32_t currentColor[NUM_LEDS];

  uint16_t pixelGroup = strip.numPixels()/4;
  for(uint16_t i=0; i < pixelGroup + 8; i++) {
       
    for(uint16_t j=0; j < 4; j++) {
      uint16_t idex = i + (j * pixelGroup);
      uint16_t upperLimit = pixelGroup * (j+1);
      uint16_t lowerLimit = pixelGroup * j;
      if (idex < upperLimit) {
        currentColor[idex] = strip.getPixelColor(idex);
      }
      if(idex >= lowerLimit && idex < upperLimit) {
        strip.setPixelColor(idex, c);
      }
      if (idex - 8 >= lowerLimit) {
        strip.setPixelColor(idex - 8, currentColor[idex - 8]);
      }
    }
    strip.show();
    delay(wait);
  }
}


void colorChase(uint32_t c, uint8_t wait) {
  uint32_t currentColor[NUM_LEDS];
  for(uint16_t i=0; i<strip.numPixels()+8; i++) {
    if (i < strip.numPixels()) {
      currentColor[i] = strip.getPixelColor(i);
    }
    strip.setPixelColor(i, c);
    if (i-8 >= 0) {
      strip.setPixelColor(i-8, currentColor[i-8]);
      
    }
    strip.show();
    delay(wait);
  }
}


void printWifiData() {
  // print your board's IP address:l
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print your MAC address:
  byte mac[6];
  WiFi.macAddress(mac);
  Serial.print("MAC address: ");
  printMacAddress(mac);
}

void printCurrentNet() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print the MAC address of the router you're attached to:
  byte bssid[6];
  WiFi.BSSID(bssid);
  Serial.print("BSSID: ");
  printMacAddress(bssid);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.println(rssi);

  // print the encryption type:
  byte encryption = WiFi.encryptionType();
  Serial.print("Encryption Type:");
  Serial.println(encryption, HEX);
  Serial.println();
}

void printMacAddress(byte mac[]) {
  for (int i = 5; i >= 0; i--) {
    if (mac[i] < 16) {
      Serial.print("0");
    }
    Serial.print(mac[i], HEX);
    if (i > 0) {
      Serial.print(":");
    }
  }
  Serial.println();
}
