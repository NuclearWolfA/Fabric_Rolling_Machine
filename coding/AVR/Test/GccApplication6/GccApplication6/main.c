#define F_CPU 16000000UL  // Define CPU frequency as 16 MHz

#include <stdlib.h> // Required for itoa()
#include <avr/io.h>
#include <util/delay.h>

// Function prototypes
void UART_Init(void);
void UART_TxChar(char ch);
unsigned char USART_Receive(void);
unsigned char USART_Available(void);
void UART_TxNumber(int number);
int USART_ReceiveNumber(void);

int main(void)
{
	// Initialize the UART
	UART_Init();

	// Set PORTB0 as output (assuming LED is connected to PORTB0)
	DDRB |= (1 << PORTB5);
	
	// Ensure the LED is off at the start
	PORTB &= ~(1 << PORTB5);

	// Clear any residual data in the receive buffer
	if (USART_Available()) {
		(void)USART_Receive();  // Read and discard the data
	}

	// Transmit a character
	UART_TxChar('A');

	// Wait a bit to ensure the transmission is complete
	_delay_ms(100);

	// Infinite loop
	while (1)
	{
		if (USART_Available())
		{
			// Read the data if available
			int num = USART_ReceiveNumber();
			UART_TxNumber(num);
			for(int i = 0; i < num; i++){
				PORTB |= (1 << PORTB5);
				_delay_ms(100);
				PORTB &= ~(1 << PORTB5);
				_delay_ms(100);
			}
		}

		// Add a delay to avoid tight polling (optional)
		_delay_ms(50);
	}
}

void UART_Init(void)
{
	// Set the baud rate (assuming 16 MHz clock and desired baud rate of 9600)
	UBRR0 = 103;

	// Enable the transmitter and receiver
	UCSR0B &= ~(1 << UCSZ02);  // Corrected bit clearing
	UCSR0B |= (1 << RXEN0) | (1 << TXEN0);

	// Set the data format to 8 data bits, no parity, 1 stop bit
	UCSR0C &= ~((1 << UMSEL00) | (1 << UMSEL01) | (1 << UPM00) | (1 << UPM01) | (1 << USBS0));
	UCSR0C |= (1 << UCSZ00) | (1 << UCSZ01);
	
	UCSR0A &= ~(1 << U2X0);
}

unsigned char USART_Receive(void)
{
	// Wait for data to be received
	while (!(UCSR0A & (1 << RXC0)));

	// Return the received data from the buffer
	return UDR0;
}

void UART_TxChar(char ch)
{
	// Wait for the transmit buffer to be empty
	while (!(UCSR0A & (1 << UDRE0)));

	// Put the data into the buffer, sending the data
	UDR0 = ch;
}

unsigned char USART_Available(void)
{
	return (UCSR0A & (1 << RXC0));  // Return non-zero if data is available
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
	char buffer[12]; // Buffer to store the received string
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
			// Store the received character, with overflow protection
			if (i < sizeof(buffer) - 1) {
				buffer[i] = received_char;
				i++;
			}
		}
	}
	
	// Convert the received string back to an integer
	return atoi(buffer); // Converts the string to an integer
}
