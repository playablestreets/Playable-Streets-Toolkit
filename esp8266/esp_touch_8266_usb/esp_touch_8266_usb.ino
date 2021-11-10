
//#include <ESP8266WebServer.h> 
//#include <AutoConnect.h>
#include <ESP8266WiFi.h>
//#include <ESP8266WiFiAP.h>
//#include <WiFiUdp.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WiFiType.h>
//#include <WiFiClientSecure.h>
#include <ESP8266WiFiSTA.h>
//#include <WiFiServer.h>
//#include <ESP8266WiFiScan.h>
//#include <ESP8266WiFiGeneric.h>
//#include <WiFiClient.h>


//ESP8266WebServer Server;          
//AutoConnect Portal(Server);

#ifdef BOARD_HAS_USB_SERIAL
#include <SLIPEncodedUSBSerial.h>
SLIPEncodedUSBSerial SLIPSerial( thisBoardsSerialUSB );
#else
#include <SLIPEncodedSerial.h>
 SLIPEncodedSerial SLIPSerial(Serial); // Change to Serial1 or Serial2 etc. for boards with multiple serial ports that don’t have Serial
#endif

//OSC stuff
//#include <WiFiUdp.h>
#include <OSCMessage.h>
#include <OSCBundle.h>
#include <OSCData.h>

#include <Wire.h>


#ifndef _BV
#define _BV(bit) (1 << (bit)) 
#endif

int out1;
int previousData1 = 0;

OSCErrorCode error;
unsigned int ledState = LOW;              // LOW means led is *on*

//WiFiUDP Udp;                                // A UDP instance to let us send and receive packets over UDP
//const IPAddress outIp(255, 255, 255, 255);     // remote IP of your computer
//const unsigned int outPort = 8000;          // remote port to receive OSC
//const unsigned int localPort = 9000;        // local port to listen for OSC packets 
//String ipString = "";
String macString = "";
String oscAddress;


#define resolution 8
#define mains 50 // 60: north america, japan; 50: most other places
#define refresh 2 * 1000000 / mains
unsigned long oldMicros = 0;
unsigned long period = 4200000 ; //70mins


void setup() {
  delay(1000);

  // pulled unused pins to low
  for(int i = 2; i < 14; i++) {
    pinMode(i, OUTPUT);
    digitalWrite(i, LOW);
  }

  for(int i = 8; i < 11; i++){
    pinMode(i, INPUT);
  }

  //Serial.begin(115200);
  //begin SLIPSerial just like Serial
  SLIPSerial.begin(115200);   // set this as high as you can reliably run on your platform

  //get mac
  byte macBytes[6];
  WiFi.macAddress(macBytes);
  char macChars[19];
  sprintf(macChars, "%02X-%02X-%02X-%02X-%02X-%02X", macBytes[0], macBytes[1], macBytes[2], macBytes[3], macBytes[4], macBytes[5]);
  macString = String(macChars);

  //Announce
  Serial.println("hey there.  I am " + macString);
  oscAddress = "/" + macString + "/touches";
  Serial.println("I will output osc to " + oscAddress + " on port 8000 as well as over serial.");

  oldMicros = micros(); 
}



void loop() {
  out1 = time(8);
  
  OSCMessage msg(oscAddress.c_str());
  msg.add( out1 );// output

  SLIPSerial.beginPacket();
  msg.send(SLIPSerial);
  SLIPSerial.endPacket();
  msg.empty();

  previousData1 = out1;  
  delay(25);
}


///////////////////////////////////read capacitance/////////////////////////////////////
long time(int pin) {
  unsigned long count = 0, total = 0;

  while((micros() - oldMicros) < refresh) {
    // pinMode is about 6 times slower than assigning
    // DDRB directly, but that pause is important
    pinMode(pin, OUTPUT);
    digitalWrite(pin,0);
    pinMode(pin, INPUT);
    while(digitalRead(pin) == 0)
      count++;
    total++;
  }
  oldMicros = micros(); 
  return (count << resolution) / total;
}
