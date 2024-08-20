/*
 * GccApplication2.c
 *
 * Created: 8/14/2024 11:18:46 AM
 * Author : Acer
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>  

#define F_CPU 16000000UL  // Define CPU frequency as 16MHz

#define dirPin1 PD4    // Corresponds to pin 4
#define stepPin1 PD5   // Corresponds to pin 5
#define dirPin2 PD6    // Corresponds to pin 6
#define stepPin2 PD7   // Corresponds to pin 7

#define stepsPerRevolution1 200
#define stepsPerRevolution2 200

volatile unsigned long timer1_overflow_count = 0;

unsigned long previousMicros1 = 0;
unsigned long previousMicros2 = 0;
int del1 = 1000;  // Microseconds for motor 1
int del2 = 1600;  // Microseconds for motor 2

void initTimer1() {
	// Initialize Timer1 to count microseconds
	TCCR1A = 0;
	TCCR1B = 0;

	// Set the prescaler to 64
	TCCR1B |= (1 << CS11) | (1 << CS10);

	// Enable Timer1 overflow interrupt
	TIMSK1 |= (1 << TOIE1);

	// Initialize the counter
	TCNT1 = 0;

	// Enable global interrupts
	sei();
}

ISR(TIMER1_OVF_vect) {
	// Increment the overflow counter
	timer1_overflow_count++;
}

unsigned long micros() {
	unsigned long m;
	uint8_t oldSREG = SREG;  // Save the global interrupt flag
	cli();                   // Disable interrupts temporarily

	// Calculate microseconds based on timer value and overflow count
	m = timer1_overflow_count;
	m = (m << 16) | TCNT1;

	SREG = oldSREG;  // Restore the global interrupt flag
	return m * (64.0 / (F_CPU / 1000000.0));
}

void setup() {
	// Set the direction pins as outputs
	DDRD |= (1 << dirPin1) | (1 << stepPin1) | (1 << dirPin2) | (1 << stepPin2);

	// Set initial direction of motors
	PORTD |= (1 << dirPin1);  // Set dirPin1 HIGH
	PORTD |= (1 << dirPin2);  // Set dirPin2 HIGH
}

void loop() {
	const long interval1 = del1;  // Microseconds for motor 1
	bool stepState1 = false;

	const long interval2 = del2;  // Microseconds for motor 2
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
				PORTD &= ~(1 << stepPin1); // Set stepPin1 LOW
			}
		}

		if (currentMicros2 - previousMicros2 >= interval2) {
			previousMicros2 = currentMicros2;

			stepState2 = !stepState2;

			if (stepState2) {
				PORTD |= (1 << stepPin2);  // Set stepPin2 HIGH
				} else {
				PORTD &= ~(1 << stepPin2); // Set stepPin2 LOW
			}
		}
	}
}

int main(void) {
	initTimer1();
	setup();

	while (1) {
		loop();
	}

	return 0;
}


