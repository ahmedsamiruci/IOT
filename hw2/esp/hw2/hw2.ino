#include <Arduino.h>
#include "program_defines.h"
#include <ESP8266WiFi.h>
extern "C" {
#include "user_interface.h"
}
#include "ESP8266TimerInterrupt.h"
#include "ESP8266_ISR_Timer.h"
#include "string.h"
#include <ArduinoJson.h>

#define LED_TOGGLE_INTERVAL_MS        2000L


bool connectToServer(void);

typedef enum {
  BLINKY_MODE_UNDEF = 0,
  BLINKY_MODE_COUNTED,
  BLINKY_MODE_INFINITY,
} enu_blinkTimerMode;


const int capacity = JSON_OBJECT_SIZE(3);

WiFiClient client;
ESP8266Timer ITimer;
ESP8266_ISR_Timer ISR_Timer;
volatile uint32_t blinkyTimerId;
volatile uint32_t dataTimerId;
volatile uint32_t blinkCount = 0;
volatile enu_blinkTimerMode blinkMode = BLINKY_MODE_UNDEF;
volatile bool bRetryConn = false;
volatile bool bSendData = false;

volatile uint32_t startMillis = 0;
volatile uint32_t connectMillis = 0;

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


void tryConnectionCb() {
  bRetryConn = true;
}

void dataTimerCb(void)
{
  //Serial.printf("dataTimerCb\n");
  bSendData = true;
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
      digitalWrite(initLED, HIGH);
      dataTimerId = ISR_Timer.setInterval(dataTimerInt, dataTimerCb);
      connectMillis = millis();
      return true;
    }
  }
}


void sendDataToServer() {
  if ( client.connected() ) {
    // Construct the sensor data as a JSON object
    StaticJsonDocument<capacity> doc;

    doc["ESP_Val"] = readSensor();


    //Serial.println("JSON Output:");
    //Print over serial
    //serializeJsonPretty(doc, Serial);
    //Serial.println();

    // send data over tcp
    serializeJsonPretty(doc, client);
    
    //String str = "{\r\n  \"ESP_Val\": 151\r\n}";
    //client.println(str);
    
  }
  else {
    //Serial.println("TCP disconencted!!!, wait 5 sec then conn");
    Serial.println("Server disconnected!!");
    connectMillis = 0;
    ISR_Timer.disable(dataTimerId);
    digitalWrite(initLED, LOW);
    digitalWrite(piLED, LOW);
    digitalWrite(espLED, LOW);
    /*ISR_Timer.setTimeout(5000, tryConnectionCb);
    startBlinkyTimer(BLINKY_MODE_INFINITY, 0);*/
  }
}

void handleRxData(String msg){
  StaticJsonDocument<capacity> doc;

  DeserializationError error = deserializeJson(doc, msg);

    // Test if parsing succeeds.
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }
   // Fetch values.
  int ESP_LED = doc["ESP_LED"];
  int Pi_LED = doc["Pi_LED"];
   
  //Serial.println("Received LEDs; ESP LED: " + String(ESP_LED) + " ; Pi LED: " + String(Pi_LED) + "\n");

  //Apply LED pattern
  digitalWrite(piLED, Pi_LED == 1? HIGH:LOW );
  digitalWrite(espLED, ESP_LED == 1? HIGH:LOW );
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  //Serial1.begin(115200); // Serial1 can TX only on pin D4
  
  while (!Serial);
  delay(100);

  Serial.printf("SDK version: %s \n", system_get_sdk_version());

  // Setup pins
  // Setup LED pins
  pinMode(initLED, OUTPUT);
  digitalWrite(initLED, LOW);

  pinMode(piLED, OUTPUT);
  digitalWrite(piLED, LOW);

  pinMode(espLED, OUTPUT);
  digitalWrite(espLED, LOW);

  initTimers();

  client.setNoDelay(true);
  
  Serial.println("Initialization Complete\n");

  setupWiFi();

  connectToServer();
}

void loop() {
  // put your main code here, to run repeatedly:
  if (bRetryConn)
  {
    bRetryConn = false;
    connectToServer();
  }

  if (bSendData)
  {
    bSendData = false;
    sendDataToServer();
  }

  // from the server, read them and print them:
  if (client.available()) {
    
    Serial.printf("Available data from server = %d bytes\n", client.available());
    String incomingData;

    while (client.available()) {
     // char c = client.read();
      incomingData += (char) client.read();
      
    }
    Serial.println("Received Msg: " + incomingData + "\n");
    
    handleRxData( incomingData );
    
  }

  // check the total connection time
  if ( client.connected() && (connectMillis > 0)) {
    // check the total connection time to initiate disconnection
    if( (millis() - connectMillis) >= disconnectTimeoutMS) {
      Serial.printf("Disconnection timeout after %d mins, disconnecting...\n", (disconnectTimeoutMS/1000/60));
      connectMillis = 0;
      //turn off all LEDs
      digitalWrite(initLED, LOW);
      digitalWrite(piLED, LOW);
      digitalWrite(espLED, LOW);
      client.stop();
    }
  }

}
