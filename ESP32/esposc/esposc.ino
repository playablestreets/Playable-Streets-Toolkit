#include <WiFi.h>
#include <WebServer.h>
#include <AutoConnect.h>

WebServer Server;          
AutoConnect Portal(Server);

#ifdef BOARD_HAS_USB_SERIAL
#include <SLIPEncodedUSBSerial.h>
SLIPEncodedUSBSerial SLIPSerial( thisBoardsSerialUSB );
#else
#include <SLIPEncodedSerial.h>
 SLIPEncodedSerial SLIPSerial(Serial); // Change to Serial1 or Serial2 etc. for boards with multiple serial ports that don’t have Serial
#endif

//OSC stuff
#include <WiFiUdp.h>
#include <OSCMessage.h>
#include <OSCBundle.h>
#include <OSCData.h>

OSCErrorCode error;
unsigned int ledState = LOW;              // LOW means led is *on*

WiFiUDP Udp;                                // A UDP instance to let us send and receive packets over UDP
const IPAddress outIp(255, 255, 255, 255);     // remote IP of your computer
const unsigned int outPort = 8000;          // remote port to receive OSC
const unsigned int localPort = 9000;        // local port to listen for OSC packets 
//String ipString = "";
String macString = "";
String oscAddress;

//touch stuff
typedef struct
{
  int pin;
  int currentValue;
  int lastValue;
}TouchInput;

#define NUMTOUCHPINS 8
TouchInput touchInputs[NUMTOUCHPINS];
int touchPins[NUMTOUCHPINS] = {4, 15, 13, 12, 14, 27, 32, 33};

void rootPage() {
  char content[] = "Hello, world";
  Server.send(200, "text/plain", content);
}

void setup() {
  delay(1000);
  Serial.begin(115200);
  //begin SLIPSerial just like Serial
  SLIPSerial.begin(115200);   // set this as high as you can reliably run on your platform
  Serial.println("setting up pins...");
//  SLIPSerial.println("setting up pins...");

  for(int i = 0; i < NUMTOUCHPINS; i++){
    touchInputs[i].pin = touchPins[i];
    touchInputs[i].currentValue = 0;
    touchInputs[i].lastValue = 1;
  }

  
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
}

void loop() {
    Portal.handleClient();
    delay(25);
    bool changeDetected = false;
    
    for(int i = 0; i < NUMTOUCHPINS; i++){
      touchInputs[i].currentValue = touchRead(touchInputs[i].pin);
      if( touchInputs[i].currentValue != touchInputs[i].lastValue ) changeDetected = true;
      touchInputs[i].lastValue = touchInputs[i].currentValue;
    }
    
    if(changeDetected){
      OSCMessage msg(oscAddress.c_str());
      for(int i = 0; i < NUMTOUCHPINS; i++) msg.add( touchInputs[i].currentValue );
      Udp.beginPacket(outIp, outPort);
      msg.send(Udp);
      Udp.endPacket();
      SLIPSerial.beginPacket();
      msg.send(SLIPSerial);
      SLIPSerial.endPacket();
      msg.empty();
    }

}
