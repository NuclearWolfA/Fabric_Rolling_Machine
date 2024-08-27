#define F_CPU 16000000UL // Adjust based on your MCU clock speed

#include <avr/io.h>
#include <util/delay.h>
#include <stdbool.h>
#include <string.h>
#include "lcd.h"
#include "keypad.h"

// Maximum string length for length input (adjust as necessary)
char length[10] = "";
char fabric;

void setup() {
	lcd_init();
	keypad_init();
	
	DDRB |= (1 << PB1); // Pin 9 (CT) as output
	TCCR1A = (1 << COM1A1) | (1 << WGM10); // Fast PWM, 8-bit
	TCCR1B = (1 << WGM12) | (1 << CS11);   // Prescaler 8
	OCR1A = 90;  // Set PWM duty cycle to 90
	
	DDRD &= ~(1 << PD0); // Pin 0 as input
	
	lcd_setCursor(0, 0);
	lcd_print("  PrecisionRoll   ");
	_delay_ms(2000);
	lcd_clear();
}

void loop() {
	lcd_print("A:Cotton B:Silk ");
	lcd_setCursor(0, 1);
	lcd_print("C:Linen  D:Other");
	
	bool fabricSel = false;
	while (!fabricSel) {
		char customKey = keypad_getKey();
		
		if (customKey == 'A' || customKey == 'B' || customKey == 'C' || customKey == 'D') {
			fabricSelect(customKey);
			
			bool confirmed = false;
			while (!confirmed) {
				customKey = keypad_getKey();
				
				if (customKey == '#') {
					confirmed = true;
					lcd_setCursor(0, 1);
					lcd_print("Length: ");
					
					bool entered = false;
					while (!entered) {
						customKey = keypad_getKey();
						if (customKey == '#') {
							entered = true;
							serial_send(fabric);
							serial_send(",");
							serial_sendln(length);
							length[0] = '\0'; // Reset length
							} else if (customKey == '*') {
							length[0] = '\0'; // Clear length input
							lcd_setCursor(0, 1);
							lcd_print("                ");
							lcd_setCursor(0, 1);
							lcd_print("Length: ");
						}

						if (customKey >= '0' && customKey <= '9') {
							strncat(length, &customKey, 1); // Append the character to length
							lcd_printChar(customKey); // Assume lcd_printChar prints one character
						}
					}
					
					fabricSel = true;
				}
			}
		}
	}

	lcd_clear();
	lcd_print("   Rolling...  ");
	
	// Check for acknowledgment on pin 0
	bool ackReceived = false;
	while (!ackReceived) {
		if (PIND & (1 << PD0)) {
			ackReceived = true;
		}
	}

	lcd_clear();
	lcd_print("    Finished.");
	_delay_ms(5000);
	lcd_clear();
}

// Example fabricSelect function
void fabricSelect(char customKey) {
	fabric = customKey;
	lcd_clear();
	switch (customKey) {
		case 'A':
		lcd_print("Cotton");
		break;
		case 'B':
		lcd_print("Silk");
		break;
		case 'C':
		lcd_print("Linen");
		break;
		case 'D':
		lcd_print("Other");
		break;
	}
}

// Example implementation of serial_send and serial_sendln
void serial_send(char data) {
	while (!(UCSR0A & (1 << UDRE0))); // Wait for the buffer to be empty
	UDR0 = data; // Transmit the data
}

void serial_sendln(const char* data) {
	while (*data) {
		serial_send(*data++);
	}
	serial_send('\n'); // Send newline at the end
}
