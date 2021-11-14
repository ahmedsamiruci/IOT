#ifndef __BLECONTROL__
#define __BLECONTROL__

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include <string> 


class DeviceInfo {

  public:
    DeviceInfo();
    ~DeviceInfo();

    void setupDevice();
    void setupAdverising();

    void notifyEvt(std::string msg);
    void updateStatus(std::string msg);
    void updateUTC(int utc);
    

  private:
    friend class MyServerCallbacks;

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

//void setupBLE(void);
//void loopBLE(void);


#endif //__BLECONTROL__
