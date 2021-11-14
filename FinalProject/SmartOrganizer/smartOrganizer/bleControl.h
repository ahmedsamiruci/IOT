#ifndef __BLECONTROL__
#define __BLECONTROL__

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include <string> 

class DeviceInfoCallbacks;

class DeviceInfo {

  public:
    DeviceInfo();
    ~DeviceInfo();

    void setupDevice();
    void setupAdverising();
    static void setCallbacks(DeviceInfoCallbacks* pCallBacks);

    void notifyEvt(std::string msg);
    void updateStatus(std::string msg);
    void updateUTC(uint32_t utc);

    static DeviceInfoCallbacks* m_pCallbacks;
  private:
    //friend class MyServerCallbacks;
    //friend class BLEServerCallbacks;

    BLEServer* m_pServer = NULL;
    BLEService* m_pService = NULL;
    BLECharacteristic* m_pScheduleChar = NULL;
    BLECharacteristic* m_pUTCChar = NULL;
    BLECharacteristic* m_pStatusChar = NULL;
    BLECharacteristic* m_pEVTChar = NULL;
    BLE2902*           m_pEVTDesc = NULL;
    bool m_bConnected = false;

    BLEServerCallbacks* m_pServerCallbacks;
    BLECharacteristicCallbacks* m_pCharCallbacks;
};


class DeviceInfoCallbacks {
  public:

  virtual ~DeviceInfoCallbacks();

  virtual void onScheduleUpdate(std::string value);
  virtual void onScheduleRead(void);
  virtual void onUTCUpdate(std::string utc);
  virtual void onUTCRead(void);
  virtual void onStatusRead(void);
  virtual void onEvtRead(void);
 
};

//void setupBLE(void);
//void loopBLE(void);


#endif //__BLECONTROL__
