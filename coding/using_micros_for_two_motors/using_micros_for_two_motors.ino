#define dirPin1 2
#define stepPin1 3
#define stepsPerRevolution1 200

#define dirPin2 5
#define stepPin2 4
#define stepsPerRevolution2 200

unsigned long previousMicros1 = 0;
unsigned long previousMicros2 = 0;
int del1 = 500; // Microseconds for motor 1
int del2 = 1000; // Microseconds for motor 2

void setup() {
  pinMode(stepPin1, OUTPUT);
  pinMode(dirPin1, OUTPUT);

  pinMode(stepPin2, OUTPUT);
  pinMode(dirPin2, OUTPUT);
  /*pinMode(A0,OUTPUT);
  pinMode(A1,INPUT);
  analogWrite(A0,200);*/ // test commmunication
}

void loop() {
  const long interval1 = del1; // Microseconds for motor 1
  bool stepState1 = false;

  const long interval2 = del2; // Microseconds for motor 2
  bool stepState2 = false;

  for (int i = 0; i < stepsPerRevolution1 * 1; i++) {
    unsigned long currentMicros1 = micros();
    unsigned long currentMicros2 = micros();

    if (currentMicros1 - previousMicros1 >= interval1) {
      // Save the last time stepPin1 was toggled
      previousMicros1 = currentMicros1;

      // Toggle stepState1
      stepState1 = !stepState1;

      // Set stepPin1 to HIGH or LOW based on stepState1
      digitalWrite(stepPin1, stepState1 ? HIGH : LOW);
    }

    if (currentMicros2 - previousMicros2 >= interval2) {
      // Save the last time stepPin2 was toggled
      previousMicros2 = currentMicros2;

      // Toggle stepState2
      stepState2 = !stepState2;

      // Set stepPin2 to HIGH or LOW based on stepState2
      digitalWrite(stepPin2, stepState2 ? HIGH : LOW);
    }
  }
  
}

