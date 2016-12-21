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
int pulses =0;
void startADC();
void initPulser();
void pulse(int len);

int main()
{
	sei();
    DDRB |= 1 << 5;
	startADC();
	initPulser();
	lcdInit();
	lcdDefaults();
	
    while (1) {
		int plen = round((ADC/1023.0)*65535.0);
		if (!(PIND & 1)) pulse(plen);
		lcdClear();
		char outStr[16];
		sprintf(outStr, "%u", pulses);
		lcdPutString(0, 0, "fuck the dog!");
		lcdPutString(0, 1, outStr);
		sprintf(outStr, "%u", (PIND & 1));
		lcdPutString(4, 1, outStr);
		sprintf(outStr, "%u", plen);
		lcdPutString(6, 1, outStr);
		lcdDraw();
		_delay_ms(1000);
		
    }
	return 0;
}

void startADC() {
	ADMUX |= (1 << REFS0); //Use Vcc as Reference
							//ADC0 is used as pin by default					
							//free running mode by defult
							
	ADCSRA |= 0b101; // prescaler at 32. 13 cycles per conversion. 26us per conversion
	ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADATE); //Enable ADC, start conversion, and enable autotriggering
}

void initPulser() {
	ICR1 = 0xFFFF; //set top to max
	TIMSK1 |= 1 << 1; //compare match interrupt
 	TCCR1A |= 1 << COM1A1 | 1 << WGM11; // Fast PWM non-inverting pin A, TOP at ICR1
	TCCR1B |= 1 << WGM13 | 1 << WGM12; //extended WGM bits
	DDRB |= 1 << 1;
}

void pulse(int len) {
	if (!pulseReady) {return;}
	pulseReady = 0;
	pulses++;
	OCR1A = len;
	TCNT1 = 0xFFFF;
	TCCR1B |= 1 << CS11 | 1 << CS10; //clock at clk/64
	
}

ISR (TIMER1_COMPA_vect) {
	TCCR1B &= ~(1 << CS11 | 1 << CS10); //insta stop timer
	pulseReady = 1;
}