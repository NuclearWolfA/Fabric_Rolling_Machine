#define F_CPU 16000000UL  // Define the CPU clock speed as 16 MHz

#include <avr/io.h>
#include <util/delay.h>
#include "lcd.h"  // Include your custom LCD library

int main(void) {
	// Initialize the LCD
	lcd_init();
	
	// Turn on the LCD display
	lcd_on();
	
	// Clear the display
	lcd_clear();
	
	// Set the cursor to the beginning (column 0, row 0)
	lcd_set_cursor(0, 0);
	
	// Print a string on the first line
	lcd_puts("Hello, World!");
	
	// Set the cursor to the beginning of the second line
	lcd_set_cursor(0, 1);
	
	// Print another string on the second line
	lcd_puts("LCD Test");

	// Enable cursor and blinking for visual testing
	lcd_enable_cursor();
	lcd_enable_blinking();
	_delay_ms(2000);  // Wait for 2 seconds
	
	// Scroll the display to the left
	for (int i = 0; i < 16; i++) {
		lcd_scroll_left();
		_delay_ms(300);
	}
	
	// Scroll the display to the right
	for (int i = 0; i < 16; i++) {
		lcd_scroll_right();
		_delay_ms(300);
	}
	
	// Disable cursor and blinking
	lcd_disable_cursor();
	lcd_disable_blinking();
	
	// Set the display to print from right to left
	lcd_set_right_to_left();
	lcd_clear();
	lcd_set_cursor(15, 0);  // Start from the right end of the first line
	lcd_puts("Right to Left");
	_delay_ms(2000);
	
	// Set the display to print from left to right again
	lcd_set_left_to_right();
	lcd_clear();
	lcd_set_cursor(0, 0);  // Start from the left end of the first line
	lcd_puts("Left to Right");
	_delay_ms(2000);
	
	// Loop indefinitely
	while (1) {
		// Toggle the backlight or additional tests can be added here
	}

	return 0;
}


