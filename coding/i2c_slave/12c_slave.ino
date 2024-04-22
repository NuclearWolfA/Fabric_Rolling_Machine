#include <Wire.h>

#define dirPin1 5
#define stepPin1 4
#define stepsPerRevolution1 200

#define dirPin2 7
#define stepPin2 6
#define stepsPerRevolution2 200

unsigned long previousMicros1 = 0;
unsigned long previousMicros2 = 0;
int del1 = 500; // Microseconds for motor 1
int del2 = 1000; // Microseconds for motor 2

bool stepState2 = false;
bool stepState1 = false;

int LED = 13;
int x = 0;
void setup() {
  // Define the LED pin as Output
  pinMode (LED_BUILTIN, OUTPUT);
  // Start the I2C Bus as Slave on address 9
  Wire.begin(9); 
  // Attach a function to trigger when something is received.
  Wire.onReceive(receiveEvent);
  Serial.begin(9600);
  pinMode(stepPin1, OUTPUT);
  pinMode(dirPin1, OUTPUT);

  pinMode(stepPin2, OUTPUT);
  pinMode(dirPin2, OUTPUT);
}
void receiveEvent(int bytes) {
  x = Wire.read();    // read one character from the I2C
  Serial.println(x);

  
}
void loop() {

  
  del1 = x*20;
  for (int i = 0; i < stepsPerRevolution1 * 1; i++) {
    unsigned long currentMicros1 = micros();
    unsigned long currentMicros2 = micros();

    if (currentMicros1 - previousMicros1 >= del1) {
      // Save the last time stepPin1 was toggled
      previousMicros1 = currentMicros1;

      // Toggle stepState1
      stepState1 = !stepState1;

      // Set stepPin1 to HIGH or LOW based on stepState1
      digitalWrite(stepPin1, stepState1 ? HIGH : LOW);
    }

    if (currentMicros2 - previousMicros2 >= del2) {
      // Save the last time stepPin2 was toggled
      previousMicros2 = currentMicros2;

      // Toggle stepState2
      stepState2 = !stepState2;

      // Set stepPin2 to HIGH or LOW based on stepState2
      digitalWrite(stepPin2, stepState2 ? HIGH : LOW);
    }
  }
  
}

