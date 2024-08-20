#ifndef LCD_H
#define LCD_H

#include <avr/io.h>
#include <util/delay.h>

// Define LCD control pins
#define LCD_RS PB4
#define LCD_EN PB5
#define LCD_PORT PORTD
#define LCD_DDR DDRD

void lcd_init();
void lcd_command(unsigned char cmnd);
void lcd_data(unsigned char data);
void lcd_clear();
void lcd_setCursor(uint8_t row, uint8_t col);
void lcd_print(const char* str);

void lcd_init() {
    // Set LCD control pins as outputs
    DDRB |= (1 << LCD_RS) | (1 << LCD_EN);
    
    // Set LCD data pins as outputs
    LCD_DDR |= 0xF0; // Upper nibble of PORTD (PD7-PD4)
    
    // LCD initialization sequence
    lcd_command(0x02); // Initialize LCD in 4-bit mode
    lcd_command(0x28); // 2 lines, 5x7 matrix in 4-bit mode
    lcd_command(0x0C); // Display on, cursor off
    lcd_command(0x06); // Increment cursor
    lcd_command(0x01); // Clear display
    _delay_ms(2);
}

void lcd_command(unsigned char cmnd) {
    // Send upper nibble
    LCD_PORT = (LCD_PORT & 0x0F) | (cmnd & 0xF0); // Mask upper nibble
    PORTB &= ~(1 << LCD_RS); // RS = 0 for command
    PORTB |= (1 << LCD_EN);  // Enable high
    _delay_us(1);
    PORTB &= ~(1 << LCD_EN); // Enable low
    
    // Send lower nibble
    LCD_PORT = (LCD_PORT & 0x0F) | (cmnd << 4); // Mask lower nibble
    PORTB |= (1 << LCD_EN);  // Enable high
    _delay_us(1);
    PORTB &= ~(1 << LCD_EN); // Enable low
    
    _delay_ms(2);
}

void lcd_data(unsigned char data) {
    // Send upper nibble
    LCD_PORT = (LCD_PORT & 0x0F) | (data & 0xF0); // Mask upper nibble
    PORTB |= (1 << LCD_RS);  // RS = 1 for data
    PORTB |= (1 << LCD_EN);  // Enable high
    _delay_us(1);
    PORTB &= ~(1 << LCD_EN); // Enable low
    
    // Send lower nibble
    LCD_PORT = (LCD_PORT & 0x0F) | (data << 4); // Mask lower nibble
    PORTB |= (1 << LCD_EN);  // Enable high
    _delay_us(1);
    PORTB &= ~(1 << LCD_EN); // Enable low
    
    _delay_ms(2);
}

void lcd_clear() {
    lcd_command(0x01); // Clear display
    _delay_ms(2);
}

void lcd_setCursor(uint8_t row, uint8_t col) {
    uint8_t address = 0x00;

    switch(row) {
        case 0:
            address = col;
            break;
        case 1:
            address = 0x40 + col;
            break;
        // Add more rows if using a larger display
    }
    
    lcd_command(0x80 | address);
}

void lcd_print(const char* str) {
    while (*str) {
        lcd_data(*str++);
    }
}

#endif // LCD_H
