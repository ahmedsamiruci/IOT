#include "sensing.h"


static sensingCallbacks defaultCallbacks; // null object pattern
sensingCallbacks* sensing::m_psensingCallbacks = NULL;

sensing::sensing() {
  //initialize sensors here
  m_psensingCallbacks = &defaultCallbacks;
  m_tempThreshold = 0;
}

sensing::~sensing() {
  delete m_psensingCallbacks;
}


void sensing::init() {
  
}

void sensing::setCallbacks(sensingCallbacks* psensingCallbacks) {
  m_psensingCallbacks = psensingCallbacks;
}

void sensing::setTempThreshold(uint8_t tempThreshold) {
  m_tempThreshold = tempThreshold;
}

int8_t sensing::getTemp(void) {
  //return rand value for now
  return random(10, 40);
}



sensingCallbacks::~sensingCallbacks() {
  
}

void sensingCallbacks::onTempEvt(int8_t temp, enu_tempEvt tempEvt) {
  
}

void sensingCallbacks::onSlotEvt(uint8_t slotNo, enu_slotEvt slotEvt) {
  
}
