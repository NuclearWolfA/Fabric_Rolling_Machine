#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#define RS PB4    // rs = 12 -> PB4
#define EN PB3    // en = 11 -> PB3

#define ROWS 4
#define COLS 4

char length[8] = "";  // Character array for length
char fabric;
bool selected;

char hexaKeys[ROWS][COLS] = {
	{'1', '2', '3', 'A'},
	{'4', '5', '6', 'B'},
	{'7', '8', '9', 'C'},
	{'*', '0', '#', 'D'}
};

// Connections to AVR (Assume)
// Rows: PC0, PC1, PD6, PD7
// Columns: PC2, PC3, PB2, PB0

void KPsetup() {
	// Set rows as output (PC0, PC1, PD6, PD7)
	DDRC |= (1 << PC0) | (1 << PC1);   // Set PC0, PC1 as output
	DDRD |= (1 << PD6) | (1 << PD7);   // Set PD6, PD7 as output
	
	// Set columns as input with pull-up (PC2, PC3, PB2, PB0)
	DDRC &= ~((1 << PC2) | (1 << PC3)); // Set PC2, PC3 as input
	DDRB &= ~((1 << PB2) | (1 << PB0)); // Set PB2, PB0 as input
	
	PORTC |= (1 << PC2) | (1 << PC3);   // Enable pull-up on PC2, PC3
	PORTB |= (1 << PB2) | (1 << PB0);   // Enable pull-up on PB2, PB0
}

char scanKeypad() {
	// Loop through each row
	for (uint8_t row = 0; row < ROWS; row++) {
		// Set all rows to high
		PORTC |= (1 << PC0) | (1 << PC1);
		PORTD |= (1 << PD6) | (1 << PD7);

		// Set current row to low
		switch(row) {
			case 0: PORTD &= ~(1 << PD7); break;
			case 1: PORTD &= ~(1 << PD6); break;
			case 2: PORTC &= ~(1 << PC0); break;
			case 3: PORTC &= ~(1 << PC1); break;
		}

		// Check each column for a pressed key
		if (!(PINC & (1 << PC2))) return hexaKeys[row][0]; // Column 1
		if (!(PINC & (1 << PC3))) return hexaKeys[row][1]; // Column 2
		if (!(PINB & (1 << PB2))) return hexaKeys[row][2]; // Column 3
		if (!(PINB & (1 << PB0))) return hexaKeys[row][3]; // Column 4
	}

	return '\0'; // No key pressed
}

unsigned char flip_bits(unsigned char value) {
	value = ((value & 0xF0) >> 4) | ((value & 0x0F) << 4);  // Swap nibbles
	value = ((value & 0xCC) >> 2) | ((value & 0x33) << 2);  // Swap pairs
	value = ((value & 0xAA) >> 1) | ((value & 0x55) << 1);  // Swap individual bits
	return value;
}

void lcd_cmd(unsigned char cmd){
	PORTD = flip_bits((cmd & 0xF0)>>2);
	PORTB |= (1<<EN);
	PORTB &= (~(1<<RS));
	_delay_ms(2);
	PORTB &= (~(1<<EN));

	PORTD = flip_bits(((cmd << 4) & 0xF0)>>2);
	PORTB |= (1<<EN);
	PORTB &= (~(1<<RS));
	_delay_ms(2);
	PORTB &= (~(1<<EN));
}

void lcd_data(unsigned char data){
	PORTD = flip_bits((data & 0xF0)>>2);
	PORTB |= (1<<EN);
	PORTB |= (1<<RS);
	_delay_ms(2);
	PORTB &= (~(1<<EN));

	PORTD = flip_bits(((data << 4) & 0xF0)>>2);
	PORTB |= (1<<EN);
	PORTB |= (1<<RS);
	_delay_ms(2);
	PORTB &= (~(1<<EN));
}

void lcd_string(const unsigned char *str, unsigned char length){
	for (unsigned char i = 0; i < length; i++){
		lcd_data(str[i]);
	}
}

void lcd_init() {
	_delay_ms(20);           // Wait for LCD to power up
	
	// Initialization in 4-bit mode
	lcd_cmd(0x02);           // Initialize in 4-bit mode
	lcd_cmd(0x28);           // 4-bit mode, 2 lines, 5x7 dots
	lcd_cmd(0x06);           // Entry mode set: increment automatically
	lcd_cmd(0x0C);           // Display on, cursor off
	lcd_cmd(0x01);           // Clear display
	_delay_ms(2);            // Clear display requires a delay
}

void fabricSelect(char customKey){
	// Clear LCD display and print character
	fabric = customKey;
	lcd_cmd(0x01);
	lcd_cmd(0x80);
	switch (customKey){
		case 'A':
		lcd_string((const unsigned char *)"Cotton", 6);
		break;
		case 'B':
		lcd_string((const unsigned char *)"Silk", 4);
		break;
		case 'C':
		lcd_string((const unsigned char *)"Linen", 5);
		break;
		case 'D':
		lcd_string((const unsigned char *)"Other", 5);
		break;
	}
}


int main(void)
{
	DDRD |= (1<<DDD2)|(1<<DDD3)|(1<<DDD4)|(1<<DDD5);
	DDRB |= (1<<DDB3)|(1<<DDB4);
	DDRB |= (1 << DDB1);

	// Set Timer1 to Fast PWM mode, 8-bit
	TCCR1A |= (1 << WGM10);   // Fast PWM, 8-bit (WGM10 set to 1)
	TCCR1A |= (1 << COM1A1);  // Clear OC1A on Compare Match, set at BOTTOM (non-inverting mode)
	
	// Set Timer1 prescaler to 64 (assuming 16 MHz clock)
	TCCR1B |= (1 << WGM12);   // Fast PWM, 8-bit (WGM12 set to 1)
	TCCR1B |= (1 << CS11) | (1 << CS10);  // Prescaler set to 64
	
	// Set duty cycle to 90 (out of 255)
	OCR1A = 90;
	lcd_init();

	lcd_cmd(0x80);           // Set cursor to the first line
	lcd_string((const unsigned char *)"  PrecisionRoll   ",17);  // Print string on the LCD
	_delay_ms(2000);         // Delay for 2 seconds
	lcd_cmd(0x01);           // Clear display
	_delay_ms(2);            // Clear display requires a delay

	KPsetup();

	while (1)
	{
		strcpy(length, "");
		selected = false;
		lcd_cmd(0x80);
		lcd_string((const unsigned char *)"A:Cotton B:Silk ", 17);
		lcd_cmd(0xC0);
		lcd_string((const unsigned char *)"C:Linen  D:Other", 17);

		while (1) {
			char key = scanKeypad();
			if (key != '\0') {
				if (key == 'A' || key == 'B' || key == 'C' || key == 'D'){
					fabricSelect(key);
					selected = true;
				}
				if ((key == '#') && (selected)) {
					break;
				}
			}
			_delay_ms(100);
		}
		_delay_ms(100);
		lcd_cmd(0xC0);
		lcd_string((const unsigned char *)"Length: ", 8);

		while (1) {
			char lenkey = scanKeypad();
			if (lenkey != '\0') {
				if (lenkey == '*') {
					strcpy(length, "");
					lcd_cmd(0xC0);
					lcd_string((const unsigned char *)"                ", 16);
					lcd_cmd(0xC0);
					lcd_string((const unsigned char *)"Length: ", 8);
					} else if ((lenkey == '#') && (strcmp(length, "") != 0)) {
					break;
					} else if ((lenkey >= '0' && lenkey <= '9')) {
					lcd_data(lenkey);
					strncat(length, &lenkey, 1);
				}
			}
			_delay_ms(200);
		}

		lcd_cmd(0x01);
		_delay_ms(2);

		lcd_cmd(0x80);
		lcd_string((const unsigned char *)"   Rolling...  ", 15);
		_delay_ms(10000);
		
		
		
		//Code for serial communication

		lcd_cmd(0x01);
		_delay_ms(2);
		lcd_string((const unsigned char *)"    Finished.", 13);
		_delay_ms(5000);
		lcd_cmd(0x01);
		_delay_ms(2);
	}
}

