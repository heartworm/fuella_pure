#include <avr/io.h>          // register definitions
#include <avr/pgmspace.h>    // storing data in program memory
#include <avr/interrupt.h>   // interrupt vectors
#include <util/delay.h>      // delay functions

#include <stdint.h>          // C header for int types like uint8_t
#include <stdbool.h>         // C header for the bool type
#include <stdio.h>         // C header for the bool type
#include <math.h>         // C header for the bool type

#include "lcd.h"

uint8_t pulseReady = 1;
uint8_t pulseLen = 128;
int pulses = 0;
void initADC();
void initPulser();
void initUART();
void pulse(int len);
void recv();

int main()
{
	DDRB |= 1 << 5;
	initADC();
	initPulser();
	initUART();
	lcdInit();
	lcdDefaults();
	
    while (1) {
		if (pulses) PORTB |= 1 << 5;
		else PORTB &= ~(1 << 5);
		recv();
/* 		lcdClear();
		char outStr[16];
		sprintf(outStr, "%u", pulseLen);
		lcdPutString(0, 0, "fuck the dog!");
		lcdPutString(0, 1, outStr);
		sprintf(outStr, "%u", pulses);
		lcdPutString(4, 1, outStr);
		lcdDraw(); */
		if (!(PIND & (1 << 0))) pulse(pulseLen);
		_delay_ms(100);
		
    }
	return 0;
}

void initADC() {
	ADMUX |= (1 << REFS0); //Use Vcc as Reference
							//ADC0 is used as pin by default					
							//free running mode by defult
							
	ADCSRA |= 0b101; // prescaler at 32. 13 cycles per conversion. 26us per conversion
	ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADATE); //Enable ADC, start conversion, and enable autotriggering
}

void initUART() {
	//set baud rate to 38400
	UBRR0 = 25;
	//enable RX, tx unnecessary
	UCSR0B |= _BV(RXEN0);
	//frame format, 8 data, with stop bit and no parity - default settings
}

void initPulser() {
	TIMSK0 |= _BV(OCIE0A); //compare match interrupt
 	TCCR0A |= _BV(COM0A1) | _BV(WGM01) | _BV(WGM00); // Fast PWM non-inverting pin 0C0A, TOP at 0xFF
	DDRD |= 1 << 6; //output (OC0A) is on PD6 
	sei();
}

void pulse(int len) {
	if (!pulseReady) {return;}
	pulseReady = 0;
	pulses++;
	OCR0A = len;
	TCNT0 = 0xFF;
	TCCR0B |= _BV(CS02) | _BV(CS00); //clock at clk/64
	
}

ISR (TIMER0_COMPA_vect) {
	TCCR0B &= ~(_BV(CS02) | _BV(CS10)); //insta stop timer
	pulses--;
	pulseReady = 1;
}

void recv() {
	if (UCSR0A & _BV(RXC0)) {
		uint8_t in = 0; //Recv'd byte
		uint8_t isValid = 0;
		while (UCSR0A & _BV(RXC0)) {
			isValid = !(UCSR0A & (_BV(FE0) | _BV(DOR0) | _BV(UPE0))); //no framing, overrun or parity errors
			in = UDR0; //clear the way for the buffer's next byte
		}
		
		if (isValid) pulseLen = in;
	}
}