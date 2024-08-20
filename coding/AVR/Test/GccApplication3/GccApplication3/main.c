#define F_CPU 16000000UL  // Clock frequency
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>



// Define the encoder pins
#define PIN_A PD2  // Connected to encoder pin A
#define PIN_B PD3  // Connected to encoder pin B

volatile int encoderPosition = 0;
volatile int lastEncoded = 0;

void setup() {
	// Set PIN_A and PIN_B as inputs
	DDRD &= ~((1 << PIN_A) | (1 << PIN_B));
	
	// Enable pull-up resistors on PIN_A and PIN_B
	PORTD |= (1 << PIN_A) | (1 << PIN_B);

	// Set up the built-in LED pin as output
	DDRB |= (1 << PB5);  // PB5 is the built-in LED on the Arduino Uno

	// Enable external interrupts on INT0 (PD2) for any logical change
	EICRA |= (1 << ISC00);
	EIMSK |= (1 << INT0);

	// Enable global interrupts
	sei();
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
	if (encoderPosition >= 10 || encoderPosition <= -10) {
		PORTB ^= (1 << PB5);  // Toggle the LED
		encoderPosition = 0;  // Reset the position
	}
}

int main(void) {
	setup();

	// Main loop
	while (1) {
		// Main loop does nothing, all work is done in ISR
	}
}


