#ifndef __SENSING__
#define __SENSING__

#include <Arduino.h>

class sensingCallbacks;

class sensing {
  public:
  sensing();
  ~sensing();

  static void init();
  static void setCallbacks(sensingCallbacks* psensingCallbacks);
  static void setTempThreshold(uint8_t tempThreshold);
  static uint8_t getTempThreshold();
  static void setTempCheckFreq(uint8_t seconds);
  static uint8_t getTempCheckFreq();
  int8_t getTemp(void);
  void loop(void);

  static sensingCallbacks*   m_psensingCallbacks;
  
  private:
  
  static uint8_t             m_freqSec;
  static uint8_t             m_tempThreshold;
  
};


typedef enum {
  TEMP_EVT_NORMAL = 0,
  TEMP_EVT_ALARM,
}enu_tempEvt;

typedef enum{
  SLOT_EVT_OPEN = 0,
  SLOT_EVT_CLOSE,
}enu_slotEvt;

class sensingCallbacks{
  public:
  virtual ~sensingCallbacks();
  
  virtual void onTempEvt(int8_t temp, enu_tempEvt tempEvt);
  virtual void onSlotEvt(uint8_t slotNo, enu_slotEvt slotEvt);
};


#endif // __SENSING__