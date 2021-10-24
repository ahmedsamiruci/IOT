#include <Arduino.h>
#include "program_defines.h"
#include <ESP8266WiFi.h>
extern "C" {
#include "user_interface.h"
}
#include "ESP8266TimerInterrupt.h"
#include "ESP8266_ISR_Timer.h"
#define LED_TOGGLE_INTERVAL_MS        2000L


bool connectToServer(void);

typedef enum {
  BLINKY_MODE_UNDEF = 0,
  BLINKY_MODE_COUNTED,
  BLINKY_MODE_INFINITY,
} enu_blinkTimerMode;


WiFiClient client;
ESP8266Timer ITimer;
ESP8266_ISR_Timer ISR_Timer;
volatile uint32_t blinkyTimerId;
volatile uint32_t blinkCount = 0;
volatile enu_blinkTimerMode blinkMode = BLINKY_MODE_UNDEF;
volatile bool bRetryConn = false;

volatile uint32_t startMillis = 0;

void IRAM_ATTR TimerHandler()
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

void startBlinkyTimer(enu_blinkTimerMode mode, uint32_t count)
{
  blinkMode = mode;
  blinkCount = count;
  ISR_Timer.enable(blinkyTimerId);
}

void stopBlinkyTimer(void) {
  blinkMode = BLINKY_MODE_INFINITY;
  blinkCount = 0;
  ISR_Timer.disable(blinkyTimerId);
}

void setupWiFi(void) {

  WiFi.begin(ssid, pswd);

  startBlinkyTimer(BLINKY_MODE_INFINITY, 0);

  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());
}


void IRAM_ATTR blinkLedCb(void) {
  static int32_t count = 0;

  switch ( blinkMode ) {
    case BLINKY_MODE_COUNTED:
      {
        count++;
        if ((blinkCount - count) >= 0) {
          digitalWrite(initLED, !digitalRead(initLED));
        }
        else {
          // Turn off LED
          digitalWrite(initLED, LOW);
          // reset the counters
          count = 0;
          blinkCount = 0;
          //stop timer
          ISR_Timer.disable(blinkyTimerId);
        }
        break;
      }

    case BLINKY_MODE_INFINITY:
      {
        digitalWrite(initLED, !digitalRead(initLED));
        break;
      }

    default:
      Serial.printf("Error in starting the timer\n");
      //stop timer
      ISR_Timer.disable(blinkyTimerId);
  }
}

void initTimers(void) {
  // Interval in microsecs
  if (ITimer.attachInterruptInterval(HW_TIMER_INTERVAL_US, TimerHandler))
  {
    startMillis = millis();
    Serial.print(F("Starting ITimer OK, millis() = ")); Serial.println(startMillis);
  }
  else
    Serial.println(F("Can't set ITimer. Select another freq. or timer"));


  blinkyTimerId = ISR_Timer.setInterval(200, blinkLedCb);
  ISR_Timer.disable(blinkyTimerId);
}


void tryConnectionCb(){
  bRetryConn = true;
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  while (!Serial);
  delay(100);

  Serial.printf("SDK version: %s \n", system_get_sdk_version());

  // Setup pins
  // Setup LED pins
  pinMode(redLED, OUTPUT);
  digitalWrite(redLED, LOW);

  pinMode(greenLED, OUTPUT);
  digitalWrite(greenLED, LOW);

  pinMode(blueLED, OUTPUT);
  digitalWrite(blueLED, LOW);

  initTimers();

  Serial.println("Initialization Complete\n");

  setupWiFi();

  connectToServer();
 
}


unsigned int readSensor() {
  int sensorVal = 0;

#ifdef MOCK_SENSOR
  sensorVal = random(200);
#else
  sensorVal = analogRead(photoSensor);
#endif

  Serial.printf("New Sensor Value = %d\n", sensorVal);
  return sensorVal;
}

bool connectToServer(void) {
  Serial.printf("Connection status: %d\n", client.connected());
  Serial.printf("Connecting to (%s):%d\n", hostIP, hostTcpPort);

  if (0 == client.connected()) {
    if (!client.connect(hostIP, hostTcpPort)) {
      Serial.printf("Connection failed, Wait for 5 sec then connect again\n");
      ISR_Timer.setTimeout(5000, tryConnectionCb);
      startBlinkyTimer(BLINKY_MODE_INFINITY, 0);
      return false;
    }
    else {
      Serial.printf("Connected to Server {local port: %d}\n", client.localPort());
      //stop the timer
      stopBlinkyTimer();
      digitalWrite(initLED, LOW);
      return true;
    }
  }
}

void sendTcpData(void)
{

  /*
    // This will send the request to the server
    client.println("hello from ESP8266");

    Serial.printf("Connection status: %d\n", client.connected());

    //read back one line from server
    Serial.println("receiving from remote server");
    //String line = client.readStringUntil('!');
    //Serial.println(line);

    //Serial.println("closing connection");
    //client.stop();

    //Serial.println("wait 5 sec...");
    //delay(5000);

    while (client.available() == 0);

    Serial.println("Data Received...");

    while (client.available()) {
      char ch = static_cast<char>(client.read());
      Serial.print(ch);
    }

    Serial.println();

    Serial.println("wait 5 sec...");
    delay(5000);

  */
}


void loop() {
  // put your main code here, to run repeatedly:
  if(bRetryConn)
  {
    bRetryConn = false;
    connectToServer();
  }
}
