
#include "bleControl.h"
#include <string> 
#include <ArduinoJson.h>


const int capacity = JSON_OBJECT_SIZE(3);

DeviceInfo bleDevice;

void setup() {
  Serial.begin(115200);
  while(!Serial);
  delay(200);

  Serial.println("Smart Organizer Firmware Started");

  bleDevice.setupDevice();

  bleDevice.setupAdverising();
}

void loop() {
  // put your main code here, to run repeatedly:
  static int count = 0;
  delay(2000);
  Serial.printf("Count = %d\n", count);
  StaticJsonDocument<capacity> doc;
  doc["Cnt"] = count;
  std::string str;
  serializeJsonPretty(doc, str);
  bleDevice.notifyEvt(str);

  //bleDevice.notifyEvt(std::to_string(count));
  count++;
}
