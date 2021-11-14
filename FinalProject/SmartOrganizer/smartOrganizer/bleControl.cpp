
#include "bleControl.h"
#include "BLEUtils.h"


/*
Service: a84c0186-4514-11ec-81d3-0242ac130003
Schedule Char: a84c03a2-4514-11ec-81d3-0242ac130003
UTC Char: a84c049c-4514-11ec-81d3-0242ac130003
Status Char: a84c0884-4514-11ec-81d3-0242ac130003
EVT Char: a84c094c-4514-11ec-81d3-0242ac130003
a84c0a00-4514-11ec-81d3-0242ac130003
a84c0abe-4514-11ec-81d3-0242ac130003
a84c0b68-4514-11ec-81d3-0242ac130003
a84c0c1c-4514-11ec-81d3-0242ac130003
a84c0cd0-4514-11ec-81d3-0242ac130003
*/


#define SERVICE_UUID        "a84c0186-4514-11ec-81d3-0242ac130003"
#define SCHEDULE_CHAR_UUID  "a84c03a2-4514-11ec-81d3-0242ac130003"
#define UTC_CHAR_UUID       "a84c049c-4514-11ec-81d3-0242ac130003"
#define STATUS_CHAR_UUID    "a84c0884-4514-11ec-81d3-0242ac130003"
#define EVT_CHAR_UUID       "a84c094c-4514-11ec-81d3-0242ac130003"

#define CHECK_ERROR(PTR, MSG)     if((PTR==NULL)){ \
  Serial.println((MSG));\
  return;\
}\



DeviceInfoCallbacks* DeviceInfo::m_pCallbacks = NULL;

static bool bConnected = false;
static DeviceInfoCallbacks defaultCallback; //null-object-pattern
static BLE2902  descriptorObj;

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      Serial.println("Device Connected");
      bConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      Serial.println("Device Disconnected");
      bConnected = false;
    }
};


class MyCharCallbacks: public BLECharacteristicCallbacks {
    void onRead(BLECharacteristic* pCharacteristic) {
      Serial.println("onRead Evt");
      if(pCharacteristic->getUUID().equals(BLEUUID(SCHEDULE_CHAR_UUID))) {
        DeviceInfo::m_pCallbacks->onScheduleRead();
      } else if(pCharacteristic->getUUID().equals(BLEUUID(UTC_CHAR_UUID))) {
        DeviceInfo::m_pCallbacks->onUTCRead();
      } else if(pCharacteristic->getUUID().equals(BLEUUID(STATUS_CHAR_UUID))) {
        DeviceInfo::m_pCallbacks->onStatusRead();
      } else if(pCharacteristic->getUUID().equals(BLEUUID(EVT_CHAR_UUID))) {
        DeviceInfo::m_pCallbacks->onEvtRead();
      }
      else {
        Serial.printf("not handled yet|n");
      }
      
    }

    void onWrite(BLECharacteristic* pCharacteristic) {
      Serial.printf("onWrite Evt..");
      if(pCharacteristic->getUUID().equals(BLEUUID(UTC_CHAR_UUID))) {
          Serial.printf("UTC char\n");
          DeviceInfo::m_pCallbacks->onUTCUpdate(pCharacteristic->getValue());
      }
      else if (pCharacteristic->getUUID().equals(BLEUUID(SCHEDULE_CHAR_UUID))) {
        Serial.printf("Schedule char\n");
        DeviceInfo::m_pCallbacks->onScheduleUpdate(pCharacteristic->getValue());
      }
      else {
        Serial.printf("not handled yet|n");
      }
      
      //Serial.println(pCharacteristic->toString());
    }

    void onNotify(BLECharacteristic* pCharacteristic) {
      Serial.println("onNotify Evt");
      //Serial.println(pCharacteristic->toString());
    }

    void onStatus(BLECharacteristic* pCharacteristic, Status s, uint32_t code) {
      Serial.println("onNotify Evt");
      Serial.printf("Status = %d, Code = %d\n", s, code);
    }
};

static MyServerCallbacks serverCallbacks;
static MyCharCallbacks charCallbacks;

DeviceInfo::DeviceInfo() {
  m_pServerCallbacks = &serverCallbacks;
  m_pCharCallbacks = &charCallbacks;
  m_pCallbacks = &defaultCallback;
}

DeviceInfo::~DeviceInfo() {
}

void DeviceInfo::setupDevice() {

  BLEDevice::init("SmartOrganizer");
  Serial.println("Compose Service ...");
  m_pServer = BLEDevice::createServer();
  CHECK_ERROR(m_pServer, "Server is Null");
  m_pServer->setCallbacks(m_pServerCallbacks);

  m_pService = m_pServer->createService(SERVICE_UUID);
  CHECK_ERROR(m_pService, "SERVICE is Null");

  m_pScheduleChar = m_pService->createCharacteristic(
                      SCHEDULE_CHAR_UUID,
                      BLECharacteristic::PROPERTY_READ |
                      BLECharacteristic::PROPERTY_WRITE
                    );
  CHECK_ERROR(m_pScheduleChar, "Schedule is Null");
  m_pScheduleChar->setCallbacks(m_pCharCallbacks);
  m_pScheduleChar->setValue("NA");

  m_pUTCChar = m_pService->createCharacteristic(
                 UTC_CHAR_UUID,
                 BLECharacteristic::PROPERTY_READ |
                 BLECharacteristic::PROPERTY_WRITE
               );
  CHECK_ERROR(m_pUTCChar, "UTC is Null");
  m_pUTCChar->setCallbacks(m_pCharCallbacks);
  uint32_t val = 0;
  m_pUTCChar->setValue(val);


  m_pStatusChar = m_pService->createCharacteristic(
                    STATUS_CHAR_UUID,
                    BLECharacteristic::PROPERTY_READ
                  );
  CHECK_ERROR(m_pStatusChar, "Status is Null");
  m_pStatusChar->setCallbacks(m_pCharCallbacks);
  m_pStatusChar->setValue("NA");

  m_pEVTChar = m_pService->createCharacteristic(
                 EVT_CHAR_UUID,
                 BLECharacteristic::PROPERTY_READ |
                 BLECharacteristic::PROPERTY_NOTIFY
               );
  CHECK_ERROR(m_pEVTChar, "EVT is Null");
  m_pEVTDesc = &descriptorObj;
  m_pEVTChar->addDescriptor(m_pEVTDesc);
  m_pEVTChar->setCallbacks(m_pCharCallbacks);
  m_pEVTChar->setValue("NA");

  m_pService->start();
}

void DeviceInfo::setupAdverising() {
  Serial.println("Setup Advertising...");
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
}


void DeviceInfo::notifyEvt(std::string msg) {
    //Serial.printf("Connected = %d, Notification = %d\n", bConnected, m_pEVTDesc->getNotifications());
   
    // set char value regardless of the connection.
    m_pEVTChar->setValue(msg);
    if(bConnected && m_pEVTDesc->getNotifications()) {
      m_pEVTChar->notify();
    }
}

void DeviceInfo::updateEvt(std::string msg) {
  m_pEVTChar->setValue(msg);
}

void DeviceInfo::updateUTC(std::string utcStr) {
  m_pUTCChar->setValue(utcStr);
}

void DeviceInfo::updateStatus(std::string msg) {
  m_pStatusChar->setValue(msg);
}

void DeviceInfo::setCallbacks(DeviceInfoCallbacks* pCallbacks) {
  m_pCallbacks = pCallbacks;
}

DeviceInfoCallbacks::~DeviceInfoCallbacks() {
  
}

void DeviceInfoCallbacks::onScheduleUpdate(std::string value) {
  
}
void DeviceInfoCallbacks::onScheduleRead(void) {
  
}
void DeviceInfoCallbacks::onUTCUpdate(std::string utc) {
  
}
void DeviceInfoCallbacks::onUTCRead(void) {
  
}
void DeviceInfoCallbacks::onStatusRead(void) {
  
}
void DeviceInfoCallbacks::onEvtRead(void) {
  
}

/*
void setupBLE(void) {

  Serial.println("Start BLE Initialization ...");

  setupService();

  setupAdvertising();

  Serial.println("BLE Initialization complete !");

}
void loopBLE(void) {
  //Serial.println("BLE loop");
}
*/
