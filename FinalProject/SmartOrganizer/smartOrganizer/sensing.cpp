#include "sensing.h"
#include "ESP32TimerInterrupt.h"
#include "ESP32_ISR_Timer.h"


#define HW_TIMER_INTERVAL_US      10000L
#define TEMP_FREQ_SEC (10)

// Init ESP32 timer 1
ESP32Timer ITimer(1);
// Init ESP32_ISR_Timer
ESP32_ISR_Timer ISR_Timer;


static sensingCallbacks defaultCallbacks; // null object pattern
sensingCallbacks* sensing::m_psensingCallbacks = NULL;
uint8_t           sensing::m_freqSec = TEMP_FREQ_SEC;
uint8_t           sensing::m_tempThreshold = 0;


static volatile bool m_bFreqTimer = false;
static volatile uint32_t timerId;

#if USING_ESP32_S2_TIMER_INTERRUPT
void IRAM_ATTR TimerHandler(void * timerNo)
#else
void IRAM_ATTR TimerHandler()
#endif
{
#if USING_ESP32_S2_TIMER_INTERRUPT
  /////////////////////////////////////////////////////////
  // Always call this for ESP32-S2 before processing ISR
  TIMER_ISR_START(timerNo);
  /////////////////////////////////////////////////////////
#endif

  ISR_Timer.run();


#if USING_ESP32_S2_TIMER_INTERRUPT
  /////////////////////////////////////////////////////////
  // Always call this for ESP32-S2 after processing ISR
  TIMER_ISR_END(timerNo);
  /////////////////////////////////////////////////////////
#endif
}

void freqTimerCb() {
  m_bFreqTimer = true;
}

void initTimers(void) {
  // Interval in microsecs
  if (ITimer.attachInterruptInterval(HW_TIMER_INTERVAL_US, TimerHandler))
  {
    Serial.print(F("Starting ITimer OK, millis() = ")); Serial.println(millis());
  }
  else
    Serial.println(F("Can't set ITimer. Select another freq. or timer"));


  timerId = ISR_Timer.setInterval(sensing::getTempCheckFreq() * 1000, freqTimerCb);
}


sensing::sensing() {
  //initialize sensors here
  m_psensingCallbacks = &defaultCallbacks;
}

sensing::~sensing() {
  delete m_psensingCallbacks;
}


void sensing::init() {
  initTimers();
}

void sensing::setCallbacks(sensingCallbacks* psensingCallbacks) {
  m_psensingCallbacks = psensingCallbacks;
}

void sensing::setTempThreshold(uint8_t tempThreshold) {
  m_tempThreshold = tempThreshold;
}

uint8_t sensing::getTempThreshold() {
  return m_tempThreshold;
}

static int8_t readTemp(void) {
  //return rand value for now
  return random(10, 40);
}

int8_t sensing::getTemp(void) {
  return readTemp();
}

void sensing::setTempCheckFreq(uint8_t seconds) {
  m_freqSec = seconds;
  //change the periodic timer here
}

uint8_t sensing::getTempCheckFreq() {
  return m_freqSec;
}


void checkTemp(uint8_t temp) {
  Serial.printf("Cur Temp = %d\n", temp);
  // Check Temp Threshold
  if (temp >= sensing::getTempThreshold()) {
    // Generate alarm event
    sensing::m_psensingCallbacks->onTempEvt(temp, TEMP_EVT_ALARM);
  }
  else {
    sensing::m_psensingCallbacks->onTempEvt(temp, TEMP_EVT_NORMAL);
  }
}


void sensing::loop(void) {
  if (m_bFreqTimer) {
    m_bFreqTimer = false;
    checkTemp(readTemp());
  }



}

sensingCallbacks::~sensingCallbacks() {

}

void sensingCallbacks::onTempEvt(int8_t temp, enu_tempEvt tempEvt) {

}

void sensingCallbacks::onSlotEvt(uint8_t slotNo, enu_slotEvt slotEvt) {

}
