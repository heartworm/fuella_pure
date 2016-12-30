#include "injector.h"

uint8_t pulseReady = 1;

void initPulser() {
	TIMSK0 |= _BV(OCIE0A); //compare match interrupt
 	TCCR0A |= _BV(COM0A1) | _BV(WGM01) | _BV(WGM00); // Fast PWM non-inverting pin 0C0A, TOP at 0xFF
	DDRD |= 1 << 6; //output (OC0A) is on PD6 
}

void pulse(int len) {	
	if (!pulseReady) {return;}
	pulseReady = 0;
	OCR0A = len;
	TCNT0 = 0xFF;
	TCCR0B |= _BV(CS02) | _BV(CS00); //clock at clk/64
	
}

uint8_t isPulseReady() {
	return pulseReady;
}

//
ISR (TIMER0_COMPA_vect) {
	TCCR0B &= ~(_BV(CS02) | _BV(CS10)); //insta stop timer
	pulseReady = 1;
}