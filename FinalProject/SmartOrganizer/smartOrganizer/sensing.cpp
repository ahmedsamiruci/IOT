#include "sensing.h"
#include "ESP32TimerInterrupt.h"
#include "ESP32_ISR_Timer.h"
#include <ezButton.h>
#include <string>


#define HW_TIMER_INTERVAL_US      10000L
#define TEMP_FREQ_SEC (10)
#define DEBOUNCE_TIME_MS (50)

const int BUTTON_NUM = 14;

const int sat_AM_pin = 36;
const int sat_PM_pin = 39;
const int sun_AM_pin = 34;
const int sun_PM_pin = 35;
const int mon_AM_pin = 32;
const int mon_PM_pin = 33;
const int tue_AM_pin = 25;
const int tue_PM_pin = 26;
const int wed_AM_pin = 27;
const int wed_PM_pin = 14;
const int thu_AM_pin = 2;
const int thu_PM_pin = 15;
const int fri_AM_pin = 4;
const int fri_PM_pin = 0;


enu_slotEvt slotsState[BUTTON_NUM];

ezButton buttonArray[] = {
  ezButton(sat_AM_pin),    //slot#0
  ezButton(sat_PM_pin),    //slot#1

  ezButton(sun_AM_pin),    //slot#2
  ezButton(sun_PM_pin),    //slot#3

  ezButton(mon_AM_pin),    //slot#4
  ezButton(mon_PM_pin),    //slot#5

  ezButton(tue_AM_pin),    //slot#6
  ezButton(tue_PM_pin),    //slot#7

  ezButton(wed_AM_pin),    //slot#8
  ezButton(wed_PM_pin),    //slot#9

  ezButton(thu_AM_pin),    //slot#10
  ezButton(thu_PM_pin),    //slot#11

  ezButton(fri_AM_pin),    //slot#12
  ezButton(fri_PM_pin),    //slot#13
};


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

String getSlotName(uint8_t slotNo) {
  switch (slotNo)
  {
    case 0: return "SAT-AM";
    case 1: return "SAT-PM";
    case 2: return "SUN-AM";
    case 3: return "SUN-PM";
    case 4: return "MON-AM";
    case 5: return "MON-PM";
    case 6: return "TUE-AM";
    case 7: return "TUE-PM";
    case 8: return "WED-AM";
    case 9: return "WED-PM";
    case 10: return "THU-AM";
    case 11: return "THU-PM";
    case 12: return "FRI-AM";
    case 13: return "FRI-PM";

  }
}

enu_slotEvt stateFromButton(uint8_t slotNo) {
  if (buttonArray[slotNo].isPressed()) {
    return SLOT_EVT_CLOSE;
  }
  else {
    return SLOT_EVT_OPEN;
  }
}

String slotStateToString(enu_slotEvt slotStatus) {
  return slotStatus == SLOT_EVT_CLOSE ? "CLOSE" : "OPEN";
}


void initHallSwitches(void) {
  Serial.println("Initial Slots State\n");
  for (byte i = 0; i < BUTTON_NUM; i++) {
    buttonArray[i].setDebounceTime(DEBOUNCE_TIME_MS); // set debounce time to 50 milliseconds
    slotsState[i] = stateFromButton(i);
    Serial.printf("%s slot is %s\n", getSlotName(i) , slotStateToString(slotsState[i]));
  }
  Serial.println("\n");
}

void IRAM_ATTR freqTimerCb() {
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

  initHallSwitches();
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
  ISR_Timer.changeInterval(timerId, seconds * 1000);
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


void hallSwitchLoop(void) {
  for (byte i = 0; i < BUTTON_NUM; i++)
    buttonArray[i].loop(); // MUST call the loop() function first

  for (byte i = 0; i < BUTTON_NUM; i++) {
    if (stateFromButton(i) != slotsState[i]) {
        printf(" %s slot changed from %s to %s\n", getSlotName(i), slotStateToString(slotsState[i]), slotStateToString(stateFromButton(i)));
        // change in the slot state
        slotsState[i] = stateFromButton(i);
        sensing::m_psensingCallbacks->onSlotEvt(i, slotsState[i]);
    }

  }
}

void sensing::loop(void) {
  if (m_bFreqTimer) {
    m_bFreqTimer = false;
    checkTemp(readTemp());
  }

  hallSwitchLoop();

}

sensingCallbacks::~sensingCallbacks() {

}

void sensingCallbacks::onTempEvt(int8_t temp, enu_tempEvt tempEvt) {

}

void sensingCallbacks::onSlotEvt(uint8_t slotNo, enu_slotEvt slotEvt) {

}
