#include <Arduino.h>

// Define pins for the rotary encoder
#define encoderPinA 2 // INT0
#define encoderPinB 3 // INT1

// Variables for encoder
volatile int encoderPos = 0; // Current position of the encoder
volatile boolean rotating = false; // Flag to indicate if encoder is rotating

// Variables for PD controller
int targetLength = 250; // Target length to stabilize the `encoderPos` at
float Kp = 1.0; // Proportional gain
float Kd = 0.1; // Derivative gain
int previousError = 0; // Previous error for derivative term

// Variables for motor control
#define dirPin1 5
#define stepPin1 4
#define stepsPerRevolution1 200
unsigned long previousMicros1 = 0;
int del1 = 500; // Microseconds for motor 1

bool stepState1 = false;

// Variable for length to roll received over serial
int length_to_roll = 0;
bool ready_to_start = false;

void setup() {
  // Initialize serial communication
  Serial.begin(9600);

  // Set encoder pins as inputs
  pinMode(encoderPinA, INPUT);
  pinMode(encoderPinB, INPUT);

  // Attach interrupts to the encoder pins
  attachInterrupt(digitalPinToInterrupt(encoderPinA), handleEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encoderPinB), handleEncoder, CHANGE);

  // Motor control pins
  pinMode(stepPin1, OUTPUT);
  pinMode(dirPin1, OUTPUT);
}

void loop() {
  // Wait for serial input to receive `length_to_roll`
  while (!Serial.available()) {
    // Wait until data is available
  }
  length_to_roll = Serial.parseInt(); // Read and parse integer from serial input

  // Print received length to Serial monitor
  Serial.print("Received length to roll: ");
  Serial.println(length_to_roll);

  // Acknowledge receipt of length_to_roll with '1'
  Serial.println("1");

  // Wait for start signal
  while (!Serial.available()) {
    // Wait until data is available
  }
  String startCommand = Serial.readStringUntil('\n');
  if (startCommand.equals("start")) {
    ready_to_start = true;
  }

  // Acknowledge start command with '1'
  Serial.println("1");

  // Check if ready to start motors
  if (ready_to_start) {
    // Calculate error for PD control
    int error = targetLength - encoderPos;
    int derivative = error - previousError;
    int adjustment = Kp * error + Kd * derivative;
    previousError = error;

    // Update motor control based on adjustment (example: adjust del1)
    del1 = 500 + adjustment;

    // Perform motor control based on adjusted del1
    for (int i = 0; i < stepsPerRevolution1 * length_to_roll; i++) {
      unsigned long currentMicros1 = micros();

      if (currentMicros1 - previousMicros1 >= del1) {
        // Save the last time stepPin1 was toggled
        previousMicros1 = currentMicros1;

        // Toggle stepState1
        stepState1 = !stepState1;

        // Set stepPin1 to HIGH or LOW based on stepState1
        digitalWrite(stepPin1, stepState1 ? HIGH : LOW);
      }
    }

    // Check if encoderPos has reached or exceeded length_to_roll
    if (encoderPos >= length_to_roll) {
      // Stop motor control
      ready_to_start = false; // Reset ready_to_start flag to stop further motor control
      // Additional actions to stop motor could be added here
    }

    // You can add other tasks here, the encoder updates will happen in the interrupts
    delay(100); // Adjust delay as needed based on your application
  }
}

void handleEncoder() {
  // Read the state of both encoder pins
  boolean A = digitalRead(encoderPinA);
  boolean B = digitalRead(encoderPinB);

  // Determine the direction of rotation
  if (A == B) {
    // Clockwise rotation
    encoderPos++;
  } else {
    // Counter-clockwise rotation
    encoderPos--;
  }
}
