#include <Arduino.h>
#include "program_defines.h"
#include <ESP8266WiFi.h>
extern "C" {
#include "user_interface.h"
}
#include "ESP8266TimerInterrupt.h"
#include "ESP8266_ISR_Timer.h"


#define LED_TOGGLE_INTERVAL_MS        2000L


WiFiClient client;
ESP8266Timer ITimer;
ESP8266_ISR_Timer ISR_Timer;

volatile uint32_t startMillis = 0;

void blinkLed(void);


void IRAM_ATTR TimerHandler()
{
#if USING_ESP32_S2_TIMER_INTERRUPT
  /////////////////////////////////////////////////////////
  // Always call this for ESP32-S2 before processing ISR
  TIMER_ISR_START(timerNo);
  /////////////////////////////////////////////////////////
#endif
  static int timeRun  = 0;

  ISR_Timer.run();

  //Serial.printf("ISR!\n");
  // Toggle LED every LED_TOGGLE_INTERVAL_MS = 2000ms = 2s
  /*if (++timeRun == ((LED_TOGGLE_INTERVAL_MS) / HW_TIMER_INTERVAL_US) )
    {
    timeRun = 0;

    //timer interrupt toggles pin LED_BUILTIN
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));

    }*/

#if USING_ESP32_S2_TIMER_INTERRUPT
  /////////////////////////////////////////////////////////
  // Always call this for ESP32-S2 after processing ISR
  TIMER_ISR_END(timerNo);
  /////////////////////////////////////////////////////////
#endif
}

void setupWiFi(void) {

  WiFi.begin(ssid, pswd);

  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    digitalWrite(initLED, !digitalRead(initLED));
    delay(200);
    Serial.print(".");
  }
  Serial.println();

  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());

  digitalWrite(initLED, LOW);
}

void doSomething(uint8_t indx) {
  Serial.printf("Hello Something: %d\n", indx);
}
void doSomething1(void) {
  doSomething(1);
}
void doSomething2(void) {
  doSomething(2);
}
void doSomething5(void) {
  doSomething(5);
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


  ISR_Timer.setInterval(1000, doSomething1);
  ISR_Timer.setInterval(2000, doSomething2);
  ISR_Timer.setInterval(5000, doSomething5);
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

  blinkLed();

  // setupWiFi();
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

void tcpConnection(void) {
  Serial.printf("Connection status: %d\n", client.connected());
  Serial.printf("Connecting to (%s):%d\n", hostIP, hostTcpPort);

  if (0 == client.connected()) {
    if (!client.connect(hostIP, hostTcpPort)) {
      Serial.printf("Connection failed\nWait for 5 sec...\n");
      delay(5000);
      return;
    }
    else {
      Serial.printf("Connected to Server with local port: %d\n", client.localPort());
    }
  }

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
}


void blinkLed(void) {

  for ( uint8 i = 0; i < 10; i++)
  {
    digitalWrite(initLED, !digitalRead(initLED));
    delay(200);
  }

  digitalWrite(initLED, LOW);
}

void loop() {
  // put your main code here, to run repeatedly:


}
