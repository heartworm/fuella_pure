#include <avr/io.h>          // register definitions
#include <avr/pgmspace.h>    // storing data in program memory
#include <avr/interrupt.h>   // interrupt vectors
#include <util/delay.h>      // delay functions

#include <stdint.h>          // C header for int types like uint8_t
#include <stdbool.h>         // C header for the bool type
#include <stdio.h>         // C header for the bool type
#include <math.h>         // C header for the bool type

#include "lcd.h"
#include "injector.h"

#define SENSOR_ATDC 0 		// rising/falling edge of sensor, when occurs, what angle are we at?


int rotInt = 0; // interval between rotations
int dRotInt = 0; // derivative of interval
int predRotInt = 0; // predicted interval (based on derivative)
uint8_t powerStroke = 0; //is the next TDC event before a power stroke or intake stroke?


uint8_t pulseLen = 128; //basically a UART receive buffer for now
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


ISR (TIMER0_COMPA_vect) {
	TCCR0B &= ~(_BV(CS02) | _BV(CS10)); //insta stop timer
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