
#include "bleControl.h"
#include <string> 
#include <ArduinoJson.h>
#include "Utils.h"
#include "sensing.h"

const int capacity = JSON_OBJECT_SIZE(3);

static DeviceInfo m_bleDevice;
static sensing    m_sensing;

static uint32_t m_utc = 0;


// Handle BLE Operations Callbacks
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
    m_bleDevice.updateEvt("hello from read\n");
  }

  void onUTCRead(void) {
    std::string utcStr = intToString(m_utc);
    Serial.printf("Read Utc value {as string %s}\n", utcStr);
    m_bleDevice.updateUTC(utcStr);
  }

  void onStatusRead(void) {
    m_bleDevice.updateStatus("hello from read\n");
  }
};


class sensingObjectCallbacks : public sensingCallbacks {
  void onTempEvt(int8_t temp, enu_tempEvt tempEvt) {
    Serial.printf("onTempEvt, Evt Type = %d, temp = %d\n", tempEvt, temp);
  }
  void onSlotEvt(String slotName, String slotEvt) {
    Serial.printf("onSlotEvt, SlotName = %s, slotEvt = %s\n", slotName, slotEvt);
  }
};

void setup() {
  Serial.begin(115200);
  while(!Serial);
  delay(200);

  Serial.println("Smart Organizer Firmware Started");

  // Initialize BLE Core
  m_bleDevice.setupDevice(); 
  m_bleDevice.setupAdverising();
  DeviceInfo::setCallbacks(new bleDeviceCallbacks());

  // Initialize Sensing
  sensing::init();
  sensing::setCallbacks(new sensingObjectCallbacks());
  sensing::setTempThreshold(30);
}

void loop() {
  // put your main code here, to run repeatedly:
  static uint32_t count = 0;

  m_sensing.loop();
  
  //delay(2000);
  //Serial.printf("Count = %d\n", count);
  StaticJsonDocument<capacity> doc;
  doc["Cnt"] = count;
  std::string str;
  serializeJsonPretty(doc, str);
  m_bleDevice.notifyEvt(str);

  m_bleDevice.updateUTC(intToString(m_utc));
  m_utc++;
  //m_bleDevice.notifyEvt(std::to_string(count));
  count++;
}
