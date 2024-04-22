#include "Adafruit_VL53L0X.h"
#include "Schedular.h"

Adafruit_VL53L0X lox = Adafruit_VL53L0X();

#define dirPin 2
#define stepPin 3
#define stepsPerRevolution 200
int del = 500;

void rotate(){
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(del);
 
    digitalWrite(stepPin, LOW);
    delayMicroseconds(del);


}

void setup() {
  Serial.begin(115200);
  
  // Declare pins as output:
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  
  while (! Serial) {
    delay(1);
  }
  
  Serial.println("Adafruit VL53L0X test");
  if (!lox.begin()) {
    Serial.println(F("Failed to boot VL53L0X"));
    while(1);
  }
  
  Serial.println(F("VL53L0X API Simple Ranging example\n\n")); 
  Scheduler.startLoop(rotate, 1);
}
void loop(){
  Schedular.execute();
}
