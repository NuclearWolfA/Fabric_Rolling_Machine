#include "Adafruit_VL53L0X.h"

Adafruit_VL53L0X lox = Adafruit_VL53L0X();

#define dirPin 2
#define stepPin 3
#define stepsPerRevolution 200
int del = 500;

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
}

void loop() {
  static unsigned long previousMillis = 0; // Variable to store the previous time
  VL53L0X_RangingMeasurementData_t measure;

  // Set the spinning direction clockwise:
  digitalWrite(dirPin, LOW);
  
  // Set the delay duration in milliseconds

  // Current time
  unsigned long currentMillis = millis();

  // Check if it's time to update the delay
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(del);
 
    digitalWrite(stepPin, LOW);
    delayMicroseconds(del);

  // Reading a measurement
  Serial.print("Reading a measurement... ");
  lox.rangingTest(&measure, false); // pass in 'true' to get debug data printout!

  if (measure.RangeStatus != 4) {  // phase failures have incorrect data
    Serial.print("Distance (mm): "); 
    Serial.println(measure.RangeMilliMeter);
    // Update the delay based on the distance measured
    del = measure.RangeMilliMeter;
  } else {
    Serial.println(" out of range ");
  }
}
