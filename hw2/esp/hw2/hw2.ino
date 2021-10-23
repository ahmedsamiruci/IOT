#include <Arduino.h>
#include "program_defines.h"
#include <ESP8266WiFi.h>
extern "C" {
#include "user_interface.h"
}

const char* ssid = "Ahmed Google";
const char* pswd = "P@ssw0rd01062602900";

const char* hostIP = "192.168.86.99";
const uint16_t hostTcpPort = 10000;


void setupWiFi(void){

  WiFi.begin(ssid, pswd);

  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
   
  Serial.printf("SDK version: %s \n", system_get_sdk_version());
  
  // Setup pins

  // Setup LED pins
  pinMode(redLED, OUTPUT);
  digitalWrite(redLED, LOW);

  pinMode(greenLED, OUTPUT);
  digitalWrite(greenLED, LOW);

  pinMode(blueLED, OUTPUT);
  digitalWrite(blueLED, LOW);

  Serial.println("Initialization Complete\n");

  setupWiFi();

}

WiFiClient client;

void loop() {
  // put your main code here, to run repeatedly:

  Serial.printf("Connection status: %d\n",client.connected());
  Serial.printf("Connecting to (%s):%d\n", hostIP, hostTcpPort);

  if(0 == client.connected()){
    if(!client.connect(hostIP, hostTcpPort)){
      Serial.printf("Connection failed\nWait for 5 sec...\n");
      delay(5000);
      return;
    }
    else{
      Serial.printf("Connected to Server with local port: %d\n", client.localPort());
    }
  }

  // This will send the request to the server
  client.println("hello from ESP8266");

  Serial.printf("Connection status: %d\n",client.connected());
  
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
