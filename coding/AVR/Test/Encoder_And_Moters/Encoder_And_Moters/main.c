#define F_CPU 16000000UL  // Clock frequency
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdbool.h>

// Define the encoder pins
#define PIN_A PD2  // Connected to encoder pin A
#define PIN_B PD3  // Connected to encoder pin B

// Define the motor control pins
#define dirPin1 PD4    // Corresponds to pin 4
#define stepPin1 PD5   // Corresponds to pin 5
#define dirPin2 PD6    // Corresponds to pin 6
#define stepPin2 PD7   // Corresponds to pin 7

#define stepsPerRevolution1 200
#define stepsPerRevolution2 200

volatile int encoderPosition = 0;
volatile int lastEncoded = 0;
volatile unsigned long timer1_overflow_count = 0;

unsigned long previousMicros1 = 0;
unsigned long previousMicros2 = 0;
int del1 = 500;  // Microseconds for motor 1
int del2 = 500;  // Microseconds for motor 2

void setup() {
	// Encoder pin setup
	DDRD &= ~((1 << PIN_A) | (1 << PIN_B));  // Set PIN_A and PIN_B as inputs
	PORTD |= (1 << PIN_A) | (1 << PIN_B);    // Enable pull-up resistors on PIN_A and PIN_B

	// Built-in LED pin setup
	DDRB |= (1 << PB5);  // Set PB5 as output for the built-in LED

	// Motor control pin setup
	DDRD |= (1 << dirPin1) | (1 << stepPin1) | (1 << dirPin2) | (1 << stepPin2);
	PORTD |= (1 << dirPin1);  // Set initial direction of motor 1
	PORTD |= (1 << dirPin2);  // Set initial direction of motor 2

	// Interrupt setup for the encoder
	EICRA |= (1 << ISC00);    // Enable interrupt on any logical change for INT0 (PD2)
	EIMSK |= (1 << INT0);     // Enable INT0

	// Timer1 setup for timing intervals
	TCCR1A = 0;
	TCCR1B = 0;
	TCCR1B |= (1 << CS11) | (1 << CS10);  // Set prescaler to 64
	TIMSK1 |= (1 << TOIE1);               // Enable Timer1 overflow interrupt
	TCNT1 = 0;

	sei();  // Enable global interrupts
}

ISR(INT0_vect) {
	// Read the current state of the encoder pins
	int MSB = (PIND & (1 << PIN_A)) ? 1 : 0;  // MSB = most significant bit
	int LSB = (PIND & (1 << PIN_B)) ? 1 : 0;  // LSB = least significant bit

	int encoded = (MSB << 1) | LSB;  // Combine the two bits
	int sum = (lastEncoded << 2) | encoded;  // Sum up the old and new encoded values

	// Determine the direction and update the position
	if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) encoderPosition++;
	if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) encoderPosition--;

	lastEncoded = encoded;  // Store the current encoded value for the next iteration

	// Check if 10 steps have been completed
	if (encoderPosition % 10 == 0 && encoderPosition != 0) {
		PORTB ^= (1 << PB5);  // Toggle the LED
	}
}

ISR(TIMER1_OVF_vect) {
	timer1_overflow_count++;
}

unsigned long micros() {
	unsigned long m;
	uint8_t oldSREG = SREG;
	cli();  // Disable interrupts temporarily

	m = timer1_overflow_count;
	m = (m << 16) | TCNT1;

	SREG = oldSREG;  // Restore the global interrupt flag
	return m * (64.0 / (F_CPU / 1000000.0));
}

void loop() {
	if (encoderPosition < 20 && encoderPosition > -20) {  // Check if encoder position is within range
		const long interval1 = del1;
		const long interval2 = del2;

		bool stepState1 = false;
		bool stepState2 = false;

		for (int i = 0; i < stepsPerRevolution1 * 1; i++) {
			unsigned long currentMicros1 = micros();
			unsigned long currentMicros2 = micros();

			if (currentMicros1 - previousMicros1 >= interval1) {
				previousMicros1 = currentMicros1;

				stepState1 = !stepState1;

				if (stepState1) {
					PORTD |= (1 << stepPin1);  // Set stepPin1 HIGH
					} else {
					PORTD &= ~(1 << stepPin1);  // Set stepPin1 LOW
				}
			}

			if (currentMicros2 - previousMicros2 >= interval2) {
				previousMicros2 = currentMicros2;

				stepState2 = !stepState2;

				if (stepState2) {
					PORTD |= (1 << stepPin2);  // Set stepPin2 HIGH
					} else {
					PORTD &= ~(1 << stepPin2);  // Set stepPin2 LOW
				}
			}
		}
		} else {
		PORTD &= ~(1 << stepPin1);  // Stop motor 1
		PORTD &= ~(1 << stepPin2);  // Stop motor 2
	}
}

int main(void) {
	setup();

	while (1) {
		loop();
	}

	return 0;
}


