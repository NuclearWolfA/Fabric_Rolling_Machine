#include "Adafruit_VL53L0X.h"
Adafruit_VL53L0X lox = Adafruit_VL53L0X();

#define dirPin 2
#define stepPin 3
#define stepsPerRevolution 200

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
  // power 
  Serial.println(F("VL53L0X API Simple Ranging example\n\n")); 
}

void loop() {
  VL53L0X_RangingMeasurementData_t measure;
  // Set the spinning direction clockwise:
  digitalWrite(dirPin, LOW);
  int del = 500;
  while(true){
  // Spin the stepper motor 1 revolution slowly:
  for (int i = 0; i < stepsPerRevolution*10; i++) {
    // These four lines result in 1 step:
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(del);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(del);
    
  }
  Serial.print("Reading a measurement... ");
  lox.rangingTest(&measure, false); // pass in 'true' to get debug data printout!

  if (measure.RangeStatus != 4) {  // phase failures have incorrect data
    Serial.print("Distance (mm): "); Serial.println(measure.RangeMilliMeter);
    del = measure.RangeMilliMeter*3;
  } else {
    Serial.println(" out of range ");
  }
    }
  }

