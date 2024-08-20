#ifndef KEYPAD_H
#define KEYPAD_H

#include <avr/io.h>
#include <util/delay.h>

// Define keypad rows and columns
#define KEYPAD_PORT PORTC
#define KEYPAD_DDR DDRC
#define KEYPAD_PIN PINC

// Define row pins
#define R1 PC0
#define R2 PC1
#define R3 PC2
#define R4 PC3

// Define column pins
#define C1 PC4
#define C2 PC5
#define C3 PC6
#define C4 PC7

char keypad_getKey();
void keypad_init();

void keypad_init() {
    // Set rows as outputs and columns as inputs
    KEYPAD_DDR |= (1 << R1) | (1 << R2) | (1 << R3) | (1 << R4); // Set rows as output
    KEYPAD_DDR &= ~((1 << C1) | (1 << C2) | (1 << C3) | (1 << C4)); // Set columns as input
    
    // Enable pull-ups for columns
    KEYPAD_PORT |= (1 << C1) | (1 << C2) | (1 << C3) | (1 << C4);
}

char keypad_getKey() {
    // Define the key mapping based on the matrix
    char hexaKeys[4][4] = {
        {'D', '#', '0', '*'},
        {'C', '9', '8', '7'},
        {'B', '6', '5', '4'},
        {'A', '3', '2', '1'}
    };

    // Scan rows and read columns
    for (uint8_t row = 0; row < 4; row++) {
        // Set current row low
        KEYPAD_PORT = ~(1 << (R1 + row));

        _delay_us(2); // Allow the port to settle
        
        // Read columns
        uint8_t colValue = KEYPAD_PIN & ((1 << C1) | (1 << C2) | (1 << C3) | (1 << C4));
        
        for (uint8_t col = 0; col < 4; col++) {
            if (!(colValue & (1 << (C1 + col)))) { // If column pin is low
                return hexaKeys[row][col];
            }
        }
    }

    return '\0'; // No key pressed
}

#endif // KEYPAD_H
