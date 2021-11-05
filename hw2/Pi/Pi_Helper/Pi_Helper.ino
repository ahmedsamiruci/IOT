
#include <String.h>

//#define TEST_MODE

static volatile bool b_pinInt = false;
const uint8 photoSensor = A0;                 // Photo Sensor has to be connected to analog pin
//
// ISR for handling interrupt triggers arising from associated button switch
//
ICACHE_RAM_ATTR void button_interrupt_handler() {
  Serial.println("Pin Int\n");
  b_pinInt = true;
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial1.begin(921600);

  while(!Serial && !Serial1);
  delay(200);


  Serial.println("Serial Code Started");

  // define Interrupt Pin
   pinMode(D1, INPUT);
   attachInterrupt(digitalPinToInterrupt(D1), button_interrupt_handler, RISING);//LOW, CHANGE, RISING, FAILLING
   
}

void loop() {
  // put your main code here, to run repeatedly:
  if(b_pinInt) 
  {
     b_pinInt = false;
     int sensorVal = analogRead(photoSensor);
     char str[20] = {'v','a','l','=','0','0','0','\n'};
     itoa(sensorVal, &str[4], 10);
     Serial1.printf("%s", str);
     Serial.printf("val string: %s\n", str);
  }

#ifdef TEST_MODE
  Serial.printf("Sensor Val = %d\n", analogRead(photoSensor));
  delay(1000);
#endif
}
