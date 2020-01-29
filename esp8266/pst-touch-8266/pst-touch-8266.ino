//Libraries required:
//Autoconnect
//OSC
//Filters https://github.com/JonHub/Filters

// 4 for esp pin 2 
//using 13 for esp pin 7
#include <AutoConnect.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiAP.h>
#include <WiFiUdp.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WiFiType.h>
#include <WiFiClientSecure.h>
#include <ESP8266WiFiSTA.h>
#include <WiFiServer.h>
#include <ESP8266WiFiScan.h>
#include <ESP8266WiFiGeneric.h>
#include <WiFiClient.h>
#include <Filters.h>
//#include <ESP8266WiFi.h>          // Replace with WiFi.h for ESP32
#include <ESP8266WebServer.h>     // Replace with WebServer.h for ESP32
#include <AutoConnect.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>
#include <OSCBundle.h>
#include <OSCData.h>

#include <Wire.h>


ESP8266WebServer Server;          // Replace with WebServer for ESP32
AutoConnect      Portal(Server);

#ifdef BOARD_HAS_USB_SERIAL
#include <SLIPEncodedUSBSerial.h>
SLIPEncodedUSBSerial SLIPSerial( thisBoardsSerialUSB );
#else
#include <SLIPEncodedSerial.h>
 SLIPEncodedSerial SLIPSerial(Serial); // Change to Serial1 or Serial2 etc. for boards with multiple serial ports that don’t have Serial
#endif

WiFiUDP Udp;                                // A UDP instance to let us send and receive packets over UDP
const IPAddress outIp(255, 255, 255, 255);     // remote IP of your computer
const unsigned int outPort = 8000;          // remote port to receive OSC
const unsigned int localPort = 9000;        // local port to listen for OSC packets 
String macString = "";
String oscAddress;

////////////////////////////////setup_touch/////////////////////////////////////
#define resolution 8
#define mains 50 // 60: north america, japan; 50: most other places
#define refresh 2 * 1000000 / mains
//unsigned long startMicros;  //some global variables available anywhere in the program
unsigned long oldMicros = 0;
unsigned long period = 4200000 ; //70mins
int out1;
int previousData1 = 0;

/////////////////////////////////////Filter////////////////////////////////////////////
int Frequency = 30 ; //for power line interferance
FilterOnePole filterOneLowpass( LOWPASS, Frequency );

////////////////////////////////////////////////////////////////////////////////
void rootPage() {
  char content[] = "Hello, world";
  Server.send(200, "text/plain", content);
}

void setup() {
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
  pinMode(13, INPUT);
  
  delay(1000);
  Serial.begin(115200);
  //begin SLIPSerial just like Serial
  SLIPSerial.begin(115200);   // set this as high as you can reliably run on your platform
  Serial.println("setting up pins...");
  //  SLIPSerial.println("setting up pins..."); 
  Serial.println("done :)");
  //get mac
  byte macBytes[6];
  WiFi.macAddress(macBytes);
  char macChars[19];
  sprintf(macChars, "%02X-%02X-%02X-%02X-%02X-%02X", macBytes[0], macBytes[1], macBytes[2], macBytes[3], macBytes[4], macBytes[5]);
  macString = String(macChars);
  Serial.println("hey there.  I am " + macString);
  oscAddress = "/" + macString + "/touches";
  Serial.println("I will output osc to " + oscAddress + " on port 8000 as well as over serial.");
  Server.on("/", rootPage);
  if (Portal.begin()) {
    Serial.println("WiFi connected: " + WiFi.localIP().toString()); //this line was hanging
    Serial.println("Starting UDP");
    Udp.begin(localPort);
    Serial.println("Started!");
  }

    oldMicros = micros(); 
}

void loop() {
    Portal.handleClient();
    //delay(25);
      OSCMessage msg(oscAddress.c_str());
      out1 = time(13);
      filterOneLowpass.input (out1);
      msg.add(out1);// output
      Udp.beginPacket(outIp, outPort);
      msg.send(Udp);
      Udp.endPacket();
      SLIPSerial.beginPacket();
      msg.send(SLIPSerial);
      SLIPSerial.endPacket();
      msg.empty();
      Serial.println(out1);
}

///////////////////////////////////read capacitance/////////////////////////////////////
long time(int pin) {
  unsigned long count = 0, total = 0;
//     if((micros() - oldMicros) > period){
//        oldMicros = micros() ;
//    }
  while((micros() - oldMicros) < refresh) {
    //Serial.print("check");
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
