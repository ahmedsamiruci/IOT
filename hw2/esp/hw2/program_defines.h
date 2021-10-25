
#ifndef __PROG_DEFINES__
#define __PROG_DEFINES__


const uint8 redLED = D1;
const uint8 greenLED = D2;
const uint8 blueLED = D3;
const uint8 photoSensor = A0;                 // Photo Sensor has to be connected to analog pin
const uint8 pushBtn = D6;

const uint8 initLED = redLED;

static const int readingIntMS = 200;           // Sensor reading interval (milliseconds)
static const int calibrationIntMS = 3000;       // time needed for calibration
static const int dataTimerInt = 5000;      
static const int blinkingTimerMS = 250;
static const int blinkingWindowMS = 250;  

static const int lightDebounce = 30;

#define switched                            true // value if the button switch has been pressed
#define triggered                           true // controls interrupt handler
#define interrupt_trigger_type            RISING // interrupt triggered on a RISING input
#define debounce                              10 // time to wait in milli secs


const char* ssid = "Ahmed Google";
const char* pswd = "P@ssw0rd01062602900";

const char* hostIP = "192.168.86.99";
const uint16_t hostTcpPort = 10000;

#define HW_TIMER_INTERVAL_US      10000L

#define MOCK_SENSOR

#endif
