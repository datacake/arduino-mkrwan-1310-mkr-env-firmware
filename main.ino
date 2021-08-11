/*

  Datacake Arduino MKR WAN 1310 + MKR ENV Demo Firmware
  Made for The Things Summer Academy

*/

#include <MKRWAN.h>
#include <Arduino_MKRENV.h>
#include <CayenneLPP.h>

// Murata Module
LoRaModem modem(Serial1);

// LoRaWAN Configuration
// Get your devEUI from Murata Module
#define LORAREGION EU868
String appEui = "";
String appKey = "";

// CayenneLPP Configuration
CayenneLPP lpp(51);

void setup() {
  
  // put your setup code here, to run once:
  Serial.begin(115200);
  while (!Serial);

  // change this to your regional band (eg. US915, AS923, ...)
  if (!modem.begin(LORAREGION)) {
    Serial.println("Failed to start module");
    while (1) {}
  };

  // Connect via LoRaWAN
  int connected = modem.joinOTAA(appEui, appKey);

  // Check Connectivity
  if (!connected) {
    Serial.println("Something went wrong; are you indoor? Move near a window and retry");
    while (1) {}
  }

  // We are connected ... idle a few
  delay(1000);

  // init ENV Board
  if (!ENV.begin()) {
    Serial.println("Failed to initialize MKR ENV shield!");
    while (1);
  } 

  // All Done ... idle a few
  delay(1000);
}

void loop() {

  // Read Sensors from ENV Board
  float temperature = ENV.readTemperature();
  float humidity    = ENV.readHumidity();
  float pressure    = ENV.readPressure();
  float illuminance = ENV.readIlluminance();
  float uva         = ENV.readUVA();
  float uvb         = ENV.readUVB();
  float uvIndex     = ENV.readUVIndex();  

  // Create LPP
  lpp.reset();
  lpp.addTemperature(0, temperature);
  lpp.addRelativeHumidity(0, humidity);  
  lpp.addBarometricPressure(0, pressure);
  lpp.addLuminosity(0, illuminance);
  lpp.addTemperature(1, uva);
  lpp.addTemperature(2, uvb);
  lpp.addTemperature(3, uvIndex);

  // Send LPP Packet over LoRaWAN
  modem.beginPacket();
  modem.write(lpp.getBuffer(), lpp.getSize());
  int err = modem.endPacket(true);
  
  // Check for errors
  if (err > 0) {
    Serial.println("Message sent correctly!");
  } else {
    Serial.println("Error sending message :(");
    Serial.println("(you may send a limited amount of messages per minute, depending on the signal strength");
    Serial.println("it may vary from 1 message every couple of seconds to 1 message every minute)");
  }

  // Idle 60 secs and start again
  delay(60000);
}
