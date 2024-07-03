#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdbool.h>

#define dirPin1 PD5
#define stepPin1 PD4
#define stepsPerRevolution1 200
unsigned long previousMicros1 = 0;
int del1 = 500;
bool stepState1 = false;

#define dirPin2 PD7
#define stepPin2 PD6
#define stepsPerRevolution2 200
unsigned long previousMicros2 = 0;
int del2 = 1000;
bool stepState2 = false;

#define encoderPinA PD2
#define encoderPinB PD3
volatile int encoderPos = 0;

int targetLength = 250;
float Kp = 1.0;
float Kd = 0.1;
int previousError = 0;

int length_to_roll = 0;
bool ready_to_start = false;

void USART_Init(unsigned int baud_rate) {
    unsigned int ubrr = F_CPU / 16 / baud_rate - 1;
    UBRR0H = (unsigned char)(ubrr >> 8);
    UBRR0L = (unsigned char)ubrr;
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void USART_Transmit(unsigned char data) {
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = data;
}

unsigned char USART_Receive() {
    while (!(UCSR0A & (1 << RXC0)));
    return UDR0;
}

void setup() {
    USART_Init(9600);

    DDRD &= ~(1 << encoderPinA);
    DDRD &= ~(1 << encoderPinB);
    PORTD |= (1 << encoderPinA);
    PORTD |= (1 << encoderPinB);

    EICRA |= (1 << ISC00) | (1 << ISC10);
    EIMSK |= (1 << INT0) | (1 << INT1);

    DDRD |= (1 << stepPin1) | (1 << dirPin1);
    DDRD |= (1 << stepPin2) | (1 << dirPin2);

    TCCR1B |= (1 << CS11);
}

ISR(INT0_vect) {
    handleEncoder();
}

ISR(INT1_vect) {
    handleEncoder();
}

void loop() {
    while (!(UCSR0A & (1 << RXC0)));
    length_to_roll = USART_Receive();

    USART_Transmit(length_to_roll);
    USART_Transmit('1');

    while (!(UCSR0A & (1 << RXC0)));
    unsigned char startCommand = USART_Receive();
    if (startCommand == 's') {
        ready_to_start = true;
    }

    USART_Transmit('1');

    if (ready_to_start) {
        int error = targetLength - encoderPos;
        int derivative = error - previousError;
        int adjustment = Kp * error + Kd * derivative;
        previousError = error;

        del1 = 500 + adjustment;

        for (int i = 0; i < stepsPerRevolution1 * length_to_roll; i++) {
            unsigned long currentMicros1 = micros();

            if (currentMicros1 - previousMicros1 >= del1) {
                previousMicros1 = currentMicros1;
                stepState1 = !stepState1;
                if (stepState1) {
                    PORTD |= (1 << stepPin1);
                } else {
                    PORTD &= ~(1 << stepPin1);
                }
            }
        }

        unsigned long currentMicros2 = micros();
        if (currentMicros2 - previousMicros2 >= del2) {
            previousMicros2 = currentMicros2;
            stepState2 = !stepState2;
            if (stepState2) {
                PORTD |= (1 << stepPin2);
            } else {
                PORTD &= ~(1 << stepPin2);
            }
        }

        if (encoderPos >= length_to_roll) {
            ready_to_start = false;
        }

        _delay_ms(100);
    }
}

void handleEncoder() {
    bool A = (PIND & (1 << encoderPinA)) >> encoderPinA;
    bool B = (PIND & (1 << encoderPinB)) >> encoderPinB;

    if (A == B) {
        encoderPos++;
    } else {
        encoderPos--;
    }
}
