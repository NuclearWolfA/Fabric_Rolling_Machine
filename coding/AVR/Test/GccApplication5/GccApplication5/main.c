#define F_CPU 16000000UL // 16 MHz clock frequency

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h> // For uint8_t
#include <stdlib.h>

#define CHA PD2 // Pin 2
#define CHB PD3 // Pin 3
#define CW_LED PB5 // Pin 13
#define CCW_LED PD7 // Pin 7

void UART_Init(void);
void UART_TxChar(char ch);
void UART_TxNumber(int number); // Function to transmit an integer
unsigned char USART_Receive(void);
unsigned char USART_Available(void);
int USART_ReceiveNumber(void); // Function to receive an integer

volatile int master_count = 0; // universal count
volatile uint8_t INTFLAG1 = 0; // interrupt status flag

void setup() {
	
	UART_Init();
	
	// Set CHA and CHB as inputs
	DDRD &= ~(1 << CHA); // Set PD2 as input
	DDRD &= ~(1 << CHB); // Set PD3 as input
	
	// Set CW_LED and CCW_LED as outputs
	DDRB |= (1 << CW_LED); // Set PB5 as output
	DDRD |= (1 << CCW_LED); // Set PD7 as output
	
	// Initialize LEDs to off
	PORTB &= ~(1 << CW_LED);
	PORTD &= ~(1 << CCW_LED);
	
	// Enable external interrupt INT0 on rising edge
	EICRA |= (1 << ISC01) | (1 << ISC00); // Set INT0 to trigger on rising edge
	EIMSK |= (1 << INT0); // Enable INT0 interrupt
	
	sei(); // Enable global interrupts
}

void loop() {
	while(master_count < 1000 && master_count > -1000) {
		PORTB |= (1 << CW_LED);  // Turn on CW_LED
		_delay_ms(1000);
		PORTB &= ~(1 << CW_LED);  // Turn off CW_LED
		_delay_ms(1000);

		// Transmit the master_count
		UART_TxNumber(master_count);
		_delay_ms(100);
	}
	
	// Ensure the LED toggles after the loop ends
	PORTB |= (1 << CW_LED); // Turn on CW_LED
	_delay_ms(100);
	PORTB &= ~(1 << CW_LED); // Turn off CW_LED
	_delay_ms(100);
	
	// Optionally transmit the final value of master_count as a character
	UART_TxNumber(master_count);
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

void UART_Init(void)
{
	// Set the baud rate (assuming 16 MHz clock and desired baud rate of 9600)
	UBRR0 = 103;

	// Enable the transmitter and receiver
	UCSR0B |= (1 << RXEN0) | (1 << TXEN0);

	// Set the data format to 8 data bits, no parity, 1 stop bit
	UCSR0C &= ~((1 << UMSEL00) | (1 << UMSEL01) | (1 << UPM00) | (1 << UPM01) | (1 << USBS0));
	UCSR0C |= (1 << UCSZ00) | (1 << UCSZ01);
}

unsigned char USART_Receive(void)
{
	// Wait for data to be received
	while (!(UCSR0A & (1 << RXC0)));

	// Return the received data from the buffer
	return UDR0;
}

unsigned char USART_Available(void)
{
	return (UCSR0A & (1 << RXC0));  // Return non-zero if data is available
}

void UART_TxChar(char ch)
{
	// Wait for the transmit buffer to be empty
	while (!(UCSR0A & (1 << UDRE0)));

	// Put the data into the buffer, sending the data
	UDR0 = ch;
}

void UART_TxNumber(int number) {
	char buffer[7]; // Enough to hold the string representation of the number
	itoa(number, buffer, 10); // Convert the integer to a string in base 10

	// Transmit each character in the string
	for (int i = 0; buffer[i] != '\0'; i++) {
		UART_TxChar(buffer[i]);
	}
	
	// Optionally, send a delimiter like newline or space to mark the end of the number
	UART_TxChar('\n'); // Sends a newline character
}

int USART_ReceiveNumber(void) {
	char buffer[7]; // Buffer to store the received string
	int i = 0;
	char received_char;
	
	// Receive characters until the delimiter is found
	while (1) {
		received_char = USART_Receive();
		
		if (received_char == '\n' || received_char == '\r') {
			// Delimiter found, end of number
			buffer[i] = '\0'; // Null-terminate the string
			break;
			} else {
			// Store the received character
			buffer[i] = received_char;
			i++;
		}
	}
	
	// Convert the received string back to an integer
	return atoi(buffer); // Converts the string to an integer
}
