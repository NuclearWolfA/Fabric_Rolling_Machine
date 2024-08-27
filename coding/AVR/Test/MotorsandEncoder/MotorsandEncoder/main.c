#define F_CPU 16000000UL  // 16 MHz clock frequency

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>  // For uint8_t
#include <stdlib.h>
#include <stdbool.h> // For boolean data types

#define CHA PD2  // Pin 2
#define CHB PD3  // Pin 3
#define CW_LED PB5  // Pin 13
#define CCW_LED PD7  // Pin 7

#define dirPin1 PD4  // Motor 1 direction pin
#define stepPin1 PD5  // Motor 1 step pin
#define dirPin2 PD6  // Motor 2 direction pin
#define stepPin2 PD7  // Motor 2 step pin

#define stepsPerRevolution1 200
#define stepsPerRevolution2 200

volatile int master_count = 0;  // universal count
volatile uint8_t INTFLAG1 = 0;  // interrupt status flag
volatile unsigned long timer1_overflow_count = 0;

unsigned long previousMicros1 = 0;
unsigned long previousMicros2 = 0;
int del1 = 1000;  // Microseconds for motor 1
int del2 = 1600;  // Microseconds for motor 2
char received_fabric = '\0';
int received_length = 0;

void UART_Init(void);
void UART_TxChar(char ch);
void UART_TxNumber(int number);
unsigned char USART_Receive(void);
unsigned char USART_Available(void);
int USART_ReceiveNumber(void);

void initTimer1(void);
unsigned long micros(void);
void rotateMotors(void);

void setup() {
	UART_Init();
	initTimer1();

	// Set CHA and CHB as inputs
	DDRD &= ~(1 << CHA);  // Set PD2 as input
	DDRD &= ~(1 << CHB);  // Set PD3 as input

	// Set the direction pins as outputs
	DDRD |= (1 << dirPin1) | (1 << stepPin1) | (1 << dirPin2) | (1 << stepPin2);

	// Set initial direction of motors
	PORTD &= ~(1 << dirPin1);  // Set dirPin1 HIGH
	PORTD |= (1 << dirPin2);  // Set dirPin2 HIGH

	// Enable external interrupt INT0 on rising edge
	EICRA |= (1 << ISC01) | (1 << ISC00);  // Set INT0 to trigger on rising edge
	EIMSK |= (1 << INT0);  // Enable INT0 interrupt

	sei();  // Enable global interrupts
}

void loop() {
	

	if (USART_Available()) {
		// Receive fabric type
		received_fabric = USART_Receive();
		char a = USART_Receive();
		// Skip the space character (if you include it in the transmission)
		while (a != ' ') {
			a = USART_Receive();
			};
			// Receive the length
			received_length = USART_ReceiveNumber();
		master_count=0;
	}

	// Rotate motors while the absolute value of master_count is less than received_length * 100
	while (abs(master_count) < (received_length * 52 - 41)) { // value got from calibration
		rotateMotors();
	}
	if(received_length !=0){
	UART_TxChar('A');
	UART_TxChar('\n');
	received_length=0;
	master_count=0;
	}
}

ISR(INT0_vect) {
	INTFLAG1 = 1;

	// Add 1 to count for CW
	if ((PIND & (1 << CHA)) && !(PIND & (1 << CHB))) {
		master_count++;
		PORTB |= (1 << CW_LED);
		PORTD &= ~(1 << CCW_LED);
	}

	// Subtract 1 from count for CCW
	if ((PIND & (1 << CHA)) && (PIND & (1 << CHB))) {
		master_count--;
		PORTB &= ~(1 << CW_LED);
		PORTD |= (1 << CCW_LED);
	}
}

int main(void) {
	setup();

	while (1) {
		loop();
	}

	return 0;
}

void UART_Init(void) {
	// Set the baud rate (assuming 16 MHz clock and desired baud rate of 9600)
	UBRR0 = 103;

	// Enable the transmitter and receiver
	UCSR0B |= (1 << RXEN0) | (1 << TXEN0);

	// Set the data format to 8 data bits, no parity, 1 stop bit
	UCSR0C &= ~((1 << UMSEL00) | (1 << UMSEL01) | (1 << UPM00) | (1 << UPM01) | (1 << USBS0));
	UCSR0C |= (1 << UCSZ00) | (1 << UCSZ01);
}

unsigned char USART_Receive(void) {
	// Wait for data to be received
	while (!(UCSR0A & (1 << RXC0)));

	// Return the received data from the buffer
	return UDR0;
}

unsigned char USART_Available(void) {
	return (UCSR0A & (1 << RXC0));  // Return non-zero if data is available
}

void UART_TxChar(char ch) {
	// Wait for the transmit buffer to be empty
	while (!(UCSR0A & (1 << UDRE0)));

	// Put the data into the buffer, sending the data
	UDR0 = ch;
}

void UART_TxNumber(int number) {
	char buffer[7];  // Enough to hold the string representation of the number
	itoa(number, buffer, 10);  // Convert the integer to a string in base 10

	// Transmit each character in the string
	for (int i = 0; buffer[i] != '\0'; i++) {
		UART_TxChar(buffer[i]);
	}

	// Optionally, send a delimiter like newline or space to mark the end of the number
	UART_TxChar('\n');  // Sends a newline character
}

int USART_ReceiveNumber(void) {
	char buffer[7];  // Buffer to store the received string
	int i = 0;
	char received_char;

	// Receive characters until the delimiter is found
	while (1) {
		received_char = USART_Receive();

		if (received_char == '\n' || received_char == '\r') {
			// Delimiter found, end of number
			buffer[i] = '\0';  // Null-terminate the string
			break;
			} else {
			// Store the received character
			buffer[i] = received_char;
			i++;
		}
	}

	// Convert the received string back to an integer
	return atoi(buffer);  // Converts the string to an integer
}

void initTimer1(void) {
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

unsigned long micros(void) {
	unsigned long m;
	uint8_t oldSREG = SREG;  // Save the global interrupt flag
	cli();  // Disable interrupts temporarily

	// Calculate microseconds based on timer value and overflow count
	m = timer1_overflow_count;
	m = (m << 16) | TCNT1;

	SREG = oldSREG;  // Restore the global interrupt flag
	return m * (64.0 / (F_CPU / 1000000.0));
}

void rotateMotors(void) {
	const long interval1 = del1;  // Microseconds for motor 1
	static bool stepState1 = false;

	const long interval2 = del2;  // Microseconds for motor 2
	static bool stepState2 = false;

	unsigned long currentMicros1 = micros();
	unsigned long currentMicros2 = micros();

	// Handle motor 1
	if (currentMicros1 - previousMicros1 >= interval1) {
		previousMicros1 = currentMicros1;

		stepState1 = !stepState1;

		if (stepState1) {
			PORTD |= (1 << stepPin1);  // Set stepPin1 HIGH
			} else {
			PORTD &= ~(1 << stepPin1);  // Set stepPin1 LOW
		}
	}

	// Handle motor 2
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


