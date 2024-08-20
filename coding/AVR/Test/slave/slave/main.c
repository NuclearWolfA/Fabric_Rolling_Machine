#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdlib.h>

// Motor 1
#define DIR_PIN1 PD4
#define STEP_PIN1 PD5
#define STEPS_PER_REVOLUTION1 200

// Motor 2
#define DIR_PIN2 PD6
#define STEP_PIN2 PD7
#define STEPS_PER_REVOLUTION2 200

volatile unsigned long previousMicros1 = 0;
volatile unsigned long previousMicros2 = 0;
volatile int del1 = 1500; // Microseconds for motor 1
volatile int del2 = 1000; // Microseconds for motor 2
volatile int steps1 = 0;
volatile int steps2 = 0;

// Add the missing declaration here
volatile unsigned long timer1_overflow_count = 0;  // Track Timer1 overflows

char receivedFabric[10] = "";
char receivedLength[10] = "";

// Timer1 overflow interrupt service routine
ISR(TIMER1_OVF_vect) {
	timer1_overflow_count++;
}

void timer1_init() {
	TCCR1B |= (1 << CS10); // No prescaling
	TIMSK1 |= (1 << TOIE1); // Enable Timer1 overflow interrupt
	sei(); // Enable global interrupts
}

unsigned long micros() {
	unsigned long m;
	uint8_t oldSREG = SREG;

	cli(); // Disable interrupts
	m = timer1_overflow_count;
	uint16_t t = TCNT1;
	SREG = oldSREG; // Restore interrupts

	return ((m << 16) + t) / (F_CPU / 1000000UL);
}

// Function prototypes
void USART_Init(unsigned int ubrr);
void USART_Transmit(char data);
char USART_Receive(void);
void USART_ReceiveString(char *buffer, int bufferLength);
void controlMotors(int rotations);

int main(void) {
	// Initialize USART
	USART_Init(103); // 9600 baud rate for 16 MHz clock

	// Initialize motor control pins
	DDRD |= (1 << STEP_PIN1) | (1 << DIR_PIN1) | (1 << STEP_PIN2) | (1 << DIR_PIN2);

	// Set initial direction for both motors
	PORTD &= ~(1 << DIR_PIN1);
	PORTD &= ~(1 << DIR_PIN2);

	timer1_init(); // Initialize Timer1

	while (1) {
		// Read incoming data until a newline character is found
		char incomingData[20];
		USART_ReceiveString(incomingData, sizeof(incomingData));

		// Find the comma separating fabric and length
		char *commaIndex = strchr(incomingData, ',');

		if (commaIndex != NULL) {
			// Extract fabric and length from the received data
			strncpy(receivedFabric, incomingData, commaIndex - incomingData);
			receivedFabric[commaIndex - incomingData] = '\0';
			strncpy(receivedLength, commaIndex + 1, sizeof(receivedLength) - 1);

			// Convert receivedLength to an integer
			int rotations = atoi(receivedLength);

			// If the fabric or length is valid, proceed to motor control
			if (rotations > 0) {
				controlMotors(rotations);
			}

			// Send acknowledgment to the sender Arduino
			USART_Transmit('T');
			USART_Transmit('h');
			USART_Transmit('a');
			USART_Transmit('r');
			USART_Transmit('o');
			USART_Transmit('o');
			USART_Transmit('s');
			USART_Transmit('h');
			USART_Transmit('a');
		}
	}
}

void controlMotors(int rotations) {
	// Number of steps to rotate based on the received length
	steps1 = 0;
	steps2 = 0;

	unsigned long interval1 = del1; // Microseconds for motor 1
	unsigned long interval2 = del2; // Microseconds for motor 2
	int totalSteps = rotations * 10;

	while (steps1 < totalSteps && steps2 < totalSteps) {
		unsigned long currentMicros1 = micros();
		unsigned long currentMicros2 = micros();

		if (currentMicros1 - previousMicros1 >= interval1) {
			previousMicros1 = currentMicros1;
			PORTD ^= (1 << STEP_PIN1); // Toggle STEP_PIN1
			steps1++;
		}

		if (currentMicros2 - previousMicros2 >= interval2) {
			previousMicros2 = currentMicros2;
			PORTD ^= (1 << STEP_PIN2); // Toggle STEP_PIN2
			steps2++;
		}
	}
}

void USART_Init(unsigned int ubrr) {
	UBRR0H = (unsigned char)(ubrr >> 8);
	UBRR0L = (unsigned char)ubrr;
	UCSR0B = (1 << RXEN0) | (1 << TXEN0);
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void USART_Transmit(char data) {
	while (!(UCSR0A & (1 << UDRE0)));
	UDR0 = data;
}

char USART_Receive(void) {
	while (!(UCSR0A & (1 << RXC0)));
	return UDR0;
}

void USART_ReceiveString(char *buffer, int bufferLength) {
	int i = 0;
	char receivedChar;
	while (i < bufferLength - 1) {
		receivedChar = USART_Receive();
		if (receivedChar == '\n') {
			break;
		}
		buffer[i++] = receivedChar;
	}
	buffer[i] = '\0';
}



