
#include "bleControl.h"
#include <string> 
#include <ArduinoJson.h>
#include "Utils.h"

const int capacity = JSON_OBJECT_SIZE(3);

static DeviceInfo bleDevice;
static uint32_t m_utc = 0;


class bleDeviceCallbacks : public DeviceInfoCallbacks{
  void onUTCUpdate(std::string utc) {
    Serial.printf("udpate utc str value = %s, and int = %d\n", utc.c_str(), atoi(utc.c_str()));
    m_utc = atoi(utc.c_str());
  }

  void onScheduleUpdate(std::string value) {
    Serial.printf("udpate schedule = %s\n", value.c_str());
  }

  void onEvtRead(void) {
    //TODO send actual schedule
    bleDevice.updateEvt("hello from read\n");
  }

  void onUTCRead(void) {
    std::string utcStr = intToString(m_utc);
    Serial.printf("Read Utc value {as string %s}\n", utcStr);
    bleDevice.updateUTC(utcStr);
  }

  void onStatusRead(void) {
    bleDevice.updateStatus("hello from read\n");
  }
};


void setup() {
  Serial.begin(115200);
  while(!Serial);
  delay(200);

  Serial.println("Smart Organizer Firmware Started");

  bleDevice.setupDevice();

  bleDevice.setupAdverising();

  DeviceInfo::setCallbacks(new bleDeviceCallbacks());
}

void loop() {
  // put your main code here, to run repeatedly:
  static uint32_t count = 0;
  delay(2000);
  Serial.printf("Count = %d\n", count);
  StaticJsonDocument<capacity> doc;
  doc["Cnt"] = count;
  std::string str;
  serializeJsonPretty(doc, str);
  bleDevice.notifyEvt(str);

  bleDevice.updateUTC(intToString(m_utc));
  m_utc++;
  //bleDevice.notifyEvt(std::to_string(count));
  count++;
}
