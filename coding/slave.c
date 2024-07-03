#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdbool.h>

// Define pins and variables for motor one
#define dirPin1 PD5
#define stepPin1 PD4
#define stepsPerRevolution1 200
unsigned long previousMicros1 = 0;
int del1 = 500; // Microseconds for motor 1
bool stepState1 = false;

// Define pins and variables for motor two
#define dirPin2 PD7
#define stepPin2 PD6
#define stepsPerRevolution2 200
unsigned long previousMicros2 = 0;
int del2 = 1000; // Microseconds for motor 2 (fixed speed)
bool stepState2 = false;

// Define pins for the rotary encoder
#define encoderPinA PD2 // INT0
#define encoderPinB PD3 // INT1

// Variables for encoder
volatile int encoderPos = 0; // Current position of the encoder

// Variables for PD controller
int targetLength = 250; // Target length to stabilize the `encoderPos` at
float Kp = 1.0; // Proportional gain
float Kd = 0.1; // Derivative gain
int previousError = 0; // Previous error for derivative term

// Variable for length to roll received over serial
int length_to_roll = 0;
bool ready_to_start = false;

// USART initialization function
void USART_Init(unsigned int baud_rate) {
    unsigned int ubrr = F_CPU / 16 / baud_rate - 1;
    
    // Set baud rate
    UBRR0H = (unsigned char)(ubrr >> 8);
    UBRR0L = (unsigned char)ubrr;
    
    // Enable receiver and transmitter
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);
    
    // Set frame format: 8 data bits, 1 stop bit
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

// Function to transmit a byte via USART
void USART_Transmit(unsigned char data) {
    // Wait for empty transmit buffer
    while (!(UCSR0A & (1 << UDRE0)))
        ;
    
    // Put data into buffer, sends the data
    UDR0 = data;
}

// Function to receive a byte via USART
unsigned char USART_Receive() {
    // Wait for data to be received
    while (!(UCSR0A & (1 << RXC0)))
        ;
    
    // Get and return received data from buffer
    return UDR0;
}

void setup() {
    // Initialize serial communication at 9600 baud
    USART_Init(9600);

    // Set encoder pins as inputs
    DDRD &= ~(1 << encoderPinA);
    DDRD &= ~(1 << encoderPinB);

    // Enable internal pull-up resistors for encoder pins
    PORTD |= (1 << encoderPinA);
    PORTD |= (1 << encoderPinB);

    // Enable external interrupts for encoder pins (INT0 and INT1)
    EICRA |= (1 << ISC00) | (1 << ISC10); // Set interrupt on any logical change
    EIMSK |= (1 << INT0) | (1 << INT1);   // Enable INT0 and INT1

    // Motor control pins for motor one
    DDRD |= (1 << stepPin1) | (1 << dirPin1);

    // Motor control pins for motor two
    DDRD |= (1 << stepPin2) | (1 << dirPin2);

    // Initialize Timer1 for microsecond timing
    TCCR1B |= (1 << CS11); // Set Timer1 prescaler to 8
}

ISR(INT0_vect) {
    handleEncoder();
}

ISR(INT1_vect) {
    handleEncoder();
}

void loop() {
    // Wait for serial input to receive `length_to_roll`
    while (!(UCSR0A & (1 << RXC0))) {
        // Wait until data is available
    }
    length_to_roll = USART_Receive(); // Read received data

    // Print received length to Serial monitor (if needed)
    USART_Transmit(length_to_roll);

    // Acknowledge receipt of length_to_roll with '1'
    USART_Transmit('1');

    // Wait for start signal
    while (!(UCSR0A & (1 << RXC0))) {
        // Wait until data is available
    }
    
    // Read start command
    unsigned char startCommand = USART_Receive();
    
    if (startCommand == 's') {
        ready_to_start = true;
    }

    // Acknowledge start command with '1'
    USART_Transmit('1');

    // Check if ready to start motors
    if (ready_to_start) {
        // Calculate error for PD control
        int error = targetLength - encoderPos;
        int derivative = error - previousError;
        int adjustment = Kp * error + Kd * derivative;
        previousError = error;

        // Update motor control based on adjustment (example: adjust del1)
        del1 = 500 + adjustment;

        // Perform motor one control based on adjusted del1
        for (int i = 0; i < stepsPerRevolution1 * length_to_roll; i++) {
            unsigned long currentMicros1 = micros();

            if (currentMicros1 - previousMicros1 >= del1) {
                // Save the last time stepPin1 was toggled
                previousMicros1 = currentMicros1;

                // Toggle stepState1
                stepState1 = !stepState1;

                // Set stepPin1 to HIGH or LOW based on stepState1
                if (stepState1) {
                    PORTD |= (1 << stepPin1);
                } else {
                    PORTD &= ~(1 << stepPin1);
                }
            }
        }

        // Perform motor two control at fixed speed
        unsigned long currentMicros2 = micros();
        if (currentMicros2 - previousMicros2 >= del2) {
            // Save the last time stepPin2 was toggled
            previousMicros2 = currentMicros2;

            // Toggle stepState2
            stepState2 = !stepState2;

            // Set stepPin2 to HIGH or LOW based on stepState2
            if (stepState2) {
                PORTD |= (1 << stepPin2);
            } else {
                PORTD &= ~(1 << stepPin2);
            }
        }

        // Check if encoderPos has reached or exceeded length_to_roll
        if (encoderPos >= length_to_roll) {
            // Stop motor control
            ready_to_start = false; // Reset ready_to_start flag to stop further motor control
            // Additional actions to stop motor could be added here
        }

        // You can add other tasks here, the encoder updates will happen in the interrupts
        _delay_ms(100); // Adjust delay as needed based on your application
    }
}

void handleEncoder() {
    // Read the state of both encoder pins
    bool A = (PIND & (1 << encoderPinA)) >> encoderPinA;
    bool B = (PIND & (1 << encoderPinB)) >> encoderPinB;

    // Determine the direction of rotation
    if (A == B) {
        // Clockwise rotation
        encoderPos++;
    } else {
        // Counter-clockwise rotation
        encoderPos--;
    }
}
