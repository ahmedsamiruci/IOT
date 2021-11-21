#include "sensing.h"
#include "ESP32TimerInterrupt.h"
#include "ESP32_ISR_Timer.h"
#include <ezButton.h>


#define HW_TIMER_INTERVAL_US      10000L
#define TEMP_FREQ_SEC (10)
#define DEBOUNCE_TIME_MS (50)

const int BUTTON_NUM = 14;

const int sat_AM_pin = 36;
const int sat_PM_pin = 39;
const int sun_AM_pin = 4;
const int sun_PM_pin = 16;
const int mon_AM_pin = 2;
const int mon_PM_pin = 15;
const int tue_AM_pin = 27;
const int tue_PM_pin = 14;
const int wed_AM_pin = 25;
const int wed_PM_pin = 26;
const int thu_AM_pin = 32;
const int thu_PM_pin = 33;
const int fri_AM_pin = 34;
const int fri_PM_pin = 35;

void onBtnChange(int pin, ezButton* pthis);

ezButton buttonArray[] = {
  ezButton(sat_AM_pin, onBtnChange),    //slot#0
  ezButton(sat_PM_pin, onBtnChange),    //slot#1

  ezButton(sun_AM_pin, onBtnChange),    //slot#2
  ezButton(sun_PM_pin, onBtnChange),    //slot#3

  ezButton(mon_AM_pin, onBtnChange),    //slot#4
  ezButton(mon_PM_pin, onBtnChange),    //slot#5

  ezButton(tue_AM_pin, onBtnChange),    //slot#6
  ezButton(tue_PM_pin, onBtnChange),    //slot#7

  ezButton(wed_AM_pin, onBtnChange),    //slot#8
  ezButton(wed_PM_pin, onBtnChange),    //slot#9

  ezButton(thu_AM_pin, onBtnChange),    //slot#10
  ezButton(thu_PM_pin, onBtnChange),    //slot#11

  ezButton(fri_AM_pin, onBtnChange),    //slot#12
  ezButton(fri_PM_pin, onBtnChange),    //slot#13
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


String getSlotName(uint8_t pinNo) {
  switch (pinNo)
  {
    case sat_AM_pin: return "SAT-AM";
    case sat_PM_pin: return "SAT-PM";
    case sun_AM_pin: return "SUN-AM";
    case sun_PM_pin: return "SUN-PM";
    case mon_AM_pin: return "MON-AM";
    case mon_PM_pin: return "MON-PM";
    case tue_AM_pin: return "TUE-AM";
    case tue_PM_pin: return "TUE-PM";
    case wed_AM_pin: return "WED-AM";
    case wed_PM_pin: return "WED-PM";
    case thu_AM_pin: return "THU-AM";
    case thu_PM_pin: return "THU-PM";
    case fri_AM_pin: return "FRI-AM";
    case fri_PM_pin: return "FRI-PM";

  }
}


String stateFromButton(ezButton* pBtn) {
  if (pBtn->isPressed()) {
    return "CLOSE";
  }
  else {
    return "OPEN";
  }
}


void onBtnChange(int pin, ezButton* pBtn) {
  //Serial.printf("Change on Btn = %d, button Closed = %s\n", pin, stateFromButton(pBtn));
  sensing::m_psensingCallbacks->onSlotEvt(getSlotName(pin), stateFromButton(pBtn));
}


void initHallSwitches(void) {
  Serial.println("Initial Slots State\n");
   
  for (byte i = 0; i < BUTTON_NUM; i++) {
    buttonArray[i].setDebounceTime(DEBOUNCE_TIME_MS); // set debounce time to 50 milliseconds
    Serial.printf("Slot %s is %s\n", getSlotName(buttonArray[i].getPin()), 
                                  (buttonArray[i].getStateRaw() == 0 ? "CLOSE": "OPEN"));
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
/*
  for (byte i = 0; i < BUTTON_NUM; i++) {
    if (stateFromButton(i) != slotsState[i]) {
        printf(" %s slot changed from %s to %s\n", getSlotName(i), slotStateToString(slotsState[i]), slotStateToString(stateFromButton(i)));
        // change in the slot state
        slotsState[i] = stateFromButton(i);
        sensing::m_psensingCallbacks->onSlotEvt(i, slotsState[i]);
    }

  }*/
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

void sensingCallbacks::onSlotEvt(String slotName, String slotEvt) {

}
