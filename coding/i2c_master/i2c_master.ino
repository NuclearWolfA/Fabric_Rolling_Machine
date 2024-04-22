#include "Adafruit_VL53L0X.h"
#include <Wire.h>
Adafruit_VL53L0X lox = Adafruit_VL53L0X();
int x = 0;
void setup() {
  // Start the I2C Bus as Master
  Wire.begin();
  Serial.begin(9600);

  // wait until serial port opens for native USB devices
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
  Wire.beginTransmission(9); // transmit to device #9
  Wire.write(x/10);              // limit x to 250 
  Wire.endTransmission();    // stop transmitting

  VL53L0X_RangingMeasurementData_t measure;
    
  Serial.print("Reading a measurement... ");
  lox.rangingTest(&measure, false); // pass in 'true' to get debug data printout!

  if (measure.RangeStatus != 4) {  // phase failures have incorrect data
    Serial.print("Distance (mm): "); Serial.println(measure.RangeMilliMeter);
    x = measure.RangeMilliMeter;
  }
  else {
    Serial.println(" out of range ");
  }
    
  delay(1000);  
}
