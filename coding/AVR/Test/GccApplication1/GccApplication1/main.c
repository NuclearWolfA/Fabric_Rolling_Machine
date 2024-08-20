/*
 * GccApplication1.c
 *
 * Created: 8/14/2024 11:17:23 AM
 * Author : Acer
 */ 

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>

#define LED_PIN PB5   // Pin 13 on the Arduino

int main(void) {
	// Set the LED pin as an output
	DDRB |= (1 << LED_PIN);

	while (1) {
		// Turn the LED on
		PORTB |= (1 << LED_PIN);
		_delay_ms(1000);  // Wait for 1000 milliseconds (1 second)

		// Turn the LED off
		PORTB &= ~(1 << LED_PIN);
		_delay_ms(1000);  // Wait for 1000 milliseconds (1 second)
	}

	return 0;
}
