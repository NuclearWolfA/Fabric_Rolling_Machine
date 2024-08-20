#include <avr/io.h>
#include <util/delay.h>
#include <lcd.h> // Include the lcd.h library
#include "vl53l0x.h" // Assuming VL53L0X is implemented at the register level

// Constants for row and column sizes
#define ROWS 4
#define COLS 4

// Array to represent keys on keypad
char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

// Connections to Arduino
uint8_t rowPins[ROWS] = {PB0, PB1, PB2, PB3};
uint8_t colPins[COLS] = {PB4, PB5, PB6, PB7};

// I2C address of the receiving device
#define I2C_ADDR 0x08

// Variables
volatile char length[16];
volatile char fabric;
volatile uint16_t measurement = 0;

// Function prototypes
void keypad_init();
char keypad_getkey();
void fabricSelect(char key);
void sendLengthSerial(const char *length);
bool receiveAckSerial();
void readAndSendMeasurement();
void i2c_init();
void i2c_start();
void i2c_stop();
void i2c_write(uint8_t data);
uint8_t i2c_read_ack();
uint8_t i2c_read_nack();
void serial_init();
void serial_send(char data);
char serial_receive();

void setup() {
  // Initialize peripherals
  lcd_init(LCD_DISP_ON); // Initialize the LCD using the lcd.h library
  keypad_init();
  i2c_init();
  serial_init();
  
  // Set up the LCD's number of columns and rows:
  lcd_puts("  PrecisionRoll   ");
  _delay_ms(2000);
  lcd_clrscr();
}

void loop() {
  lcd_puts("A:Cotton B:Silk ");
  lcd_gotoxy(0, 1); // Move to second line
  lcd_puts("C:Linen  D:Other");

  bool fabricSel = false;
  while (!fabricSel) {
    // Get key value if pressed
    char customKey = keypad_getkey();

    if (customKey == 'A' || customKey == 'B' || customKey == 'C' || customKey == 'D') {
      fabricSelect(customKey);

      bool confirmed = false;
      while (!confirmed) {
        customKey = keypad_getkey();

        if (customKey == 'A' || customKey == 'B' || customKey == 'C' || customKey == 'D') {
          fabricSelect(customKey);
        }

        if (customKey == '#') {
          confirmed = true;
        }
      }

      lcd_gotoxy(0, 1); // Move to second line
      lcd_puts("Length: ");
      bool entered = false;
      uint8_t lengthIndex = 0;
      while (!entered) {
        customKey = keypad_getkey();
        if (customKey == '#') {
          length[lengthIndex] = '\0'; // Null-terminate the string
          entered = true;
        } else if (customKey == '*') {
          lengthIndex = 0;
          lcd_gotoxy(0, 1); // Move to second line
          lcd_puts("                ");
          lcd_gotoxy(0, 1); // Move to second line
          lcd_puts("Length: ");
        }
        if ((customKey >= '0' && customKey <= '9') && lengthIndex < 15) {
          length[lengthIndex++] = customKey;
          lcd_putc(customKey);
        }
      }
      fabricSel = true;
    }
  }

  // Send length to another device via serial
  sendLengthSerial(length);

  // Wait for acknowledgment from the receiving device
  bool ackReceived = false;
  while (!ackReceived) {
    ackReceived = receiveAckSerial();
  }

  // Prompt the user to press # to start
  lcd_clrscr();
  lcd_puts("Press # to start");
  while (1) {
    char customKey = keypad_getkey();
    if (customKey == '#') {
      // Send '1' to the other Arduino board
      serial_send('1');
      break;
    }
  }

  // Clear the display and show rolling message
  lcd_clrscr();
  lcd_puts("   Rolling...  ");
  _delay_ms(10000);

  // Continuously read and send measurement while waiting for '1'
  while (1) {
    readAndSendMeasurement();
    if (UCSR0A & (1 << RXC0)) {
      char received = serial_receive();
      if (received == '1') {
        break;
      }
    }
  }

  // Clear length for next iteration
  length[0] = '\0';
}

int main(){
    setup();
    while (1)
    {
        loop()
    }
    return 0;
}

void keypad_init() {
  // Initialize keypad at register level
  DDRB &= ~((1 << PB0) | (1 << PB1) | (1 << PB2) | (1 << PB3)); // Rows as input
  DDRB |= ((1 << PB4) | (1 << PB5) | (1 << PB6) | (1 << PB7));  // Columns as output
  PORTB |= ((1 << PB0) | (1 << PB1) | (1 << PB2) | (1 << PB3)); // Enable pull-up resistors for rows
}

char keypad_getkey() {
  for (uint8_t col = 0; col < COLS; col++) {
    PORTB &= ~((1 << PB4) | (1 << PB5) | (1 << PB6) | (1 << PB7)); // Clear all columns
    PORTB |= (1 << colPins[col]); // Set current column high
    _delay_us(10); // Short delay for stabilization

    for (uint8_t row = 0; row < ROWS; row++) {
      if (!(PINB & (1 << rowPins[row]))) { // Check if key is pressed
        while (!(PINB & (1 << rowPins[row]))); // Wait for key release
        return hexaKeys[row][col];
      }
    }
  }
  return 0; // No key pressed
}

void fabricSelect(char key) {
  lcd_clrscr();
  lcd_puts("Fabric: ");
  lcd_putc(key);
  fabric = key;
}

void sendLengthSerial(const char *length) {
  while (*length) {
    serial_send(*length++);
  }
}

bool receiveAckSerial() {
  if (UCSR0A & (1 << RXC0)) {
    char ack = serial_receive();
    return (ack == '1');
  }
  return false;
}

void readAndSendMeasurement() {
  VL53L0X_RangingMeasurementData_t measure;
  vl53l0x_rangingTest(&measure, false);

  if (measure.RangeStatus != 4) {
    measurement = measure.RangeMilliMeter;
  } else {
    measurement = 0;
  }

  i2c_start();
  i2c_write(I2C_ADDR << 1);
  i2c_write(measurement & 0xFF);
  i2c_write((measurement >> 8) & 0xFF);
  i2c_stop();
}

void i2c_init() {
  TWSR = 0x00;
  TWBR = 0x48; // Set SCL frequency to 100 kHz
  TWCR = (1 << TWEN); // Enable TWI
}

void i2c_start() {
  TWCR = (1 << TWSTA) | (1 << TWEN) | (1 << TWINT);
  while (!(TWCR & (1 << TWINT)));
}

void i2c_stop() {
  TWCR = (1 << TWSTO) | (1 << TWEN) | (1 << TWINT);
  while (TWCR & (1 << TWSTO));
}

void i2c_write(uint8_t data) {
  TWDR = data;
  TWCR = (1 << TWEN) | (1 << TWINT);
  while (!(TWCR & (1 << TWINT)));
}

uint8_t i2c_read_ack() {
  TWCR = (1 << TWEN) | (1 << TWINT) | (1 << TWEA);
  while (!(TWCR & (1 << TWINT)));
  return TWDR;
}

uint8_t i2c_read_nack() {
  TWCR = (1 << TWEN) | (1 << TWINT);
  while (!(TWCR & (1 << TWINT)));
  return TWDR;
}

void serial_init() {
  UBRR0H = 0;
  UBRR0L = 103; // 9600 baud rate at 16 MHz
  UCSR0B = (1 << RXEN0) | (1 << TXEN0); // Enable receiver and transmitter
  UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // 8 data bits, 1 stop bit
}

void serial_send(char data) {
  while (!(UCSR0A & (1 << UDRE0)));
}