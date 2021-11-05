
#ifndef __PROG_DEFINES__
#define __PROG_DEFINES__


const uint8_t redLED = D1;
const uint8_t greenLED = D2;
const uint8_t blueLED = D3;
const uint8_t photoSensor = A0;                 // Photo Sensor has to be connected to analog pin
const uint8_t pushBtn = D6;

const uint8_t initLED = redLED;
const uint8_t espLED = blueLED;
const uint8_t piLED = greenLED;

static const uint32_t readingIntMS = 200;           // Sensor reading interval (milliseconds)
static const uint32_t calibrationIntMS = 3000;       // time needed for calibration
static const uint32_t dataTimerInt = 250;      
static const uint32_t blinkingTimerMS = 2000;
static const uint32_t blinkingWindowMS = 250;  
static const uint32_t disconnectTimeoutMS = 15 * 60 * 1000;


static const uint8_t lightDebounce = 30;

#define switched                            true // value if the button switch has been pressed
#define triggered                           true // controls interrupt handler
#define interrupt_trigger_type            RISING // interrupt triggered on a RISING input
#define debounce                              10 // time to wait in milli secs


const char* ssid = "Ahmed Google";
const char* pswd = "P@ssw0rd01062602900";

//const char* hostIP = "192.168.86.122";

// Pi Address
const char* hostIP = "192.168.86.25";
const uint16_t hostTcpPort = 10000;

#define HW_TIMER_INTERVAL_US      10000L

//#define MOCK_SENSOR

#endif
