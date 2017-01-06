#include <avr/io.h>          // register definitions
#include <avr/pgmspace.h>    // storing data in program memory
#include <avr/interrupt.h>   // interrupt vectors
#include <util/delay.h>      // delay functions

#include <stdint.h>          // C header for int types like uint8_t
#include <stdbool.h>    
#include <stdio.h>      
#include <math.h>       

#include "lcd.h"
#include "injector.h"
#include "serial.h"

#define SENSOR_ATDC 48.909426987061 // rising/falling edge of sensor, when occurs, what angle are we at?

uint8_t instBuf[RX_BUFLEN];

uint16_t rotInt = 0; // interval between rotations
int16_t dRotInt = 0; // derivative of interval
uint16_t predRotInt = false; // predicted interval (based on derivative)

bool engineRunning = false; //is engine currently running, enabled on first hall sensor and disabled on overflow of timer
bool engineTiming = false;
bool powerStroke = false; //is the next TDC event before a power stroke or intake stroke? (if true probs fire spark)
bool injected = false;
bool revLimit = false;

uint8_t pulseLen = 128; //basically a UART receive buffer for now

// void initADC();
void initEngineTimer();
void processFrame();
float getEngineAngle();



int main()
{
	DDRB |= 1 << 5;
	// initADC();
	initPulser();
	initEngineTimer();
	initSerial();
	//lcdInit();
	//lcdDefaults();
	
	
	
	sei(); //turn on interrupts
    while (1) {
		/* if (pulseLen == 0x33) {
			PORTB |= _BV(5);
		} else {
			PORTB &= ~_BV(5);
		} */
		
/* 		else {
			lcdClear();
				char outStr[16];
				lcdPutString(0, 0, "Run:");
				
				sprintf(outStr, "%u", engineRunning);
				lcdPutString(4, 0, outStr);
				
				sprintf(outStr, "%u", rotInt);
				lcdPutString(6, 0, outStr);
				
				if (engineTiming) {
					sprintf(outStr, "%u", ((int)getEngineAngle()));
					lcdPutString(0, 1, outStr);
				} else {
					lcdPutString(0, 1, "NTIMING");
				}
				
				sprintf(outStr, "%u", pulseLen);
				lcdPutString(7, 1, outStr);
			lcdDraw();
			_delay_ms(250);
		} */
		
		
		if (engineTiming) {
			
			//hysteresis, stop a weird oscillating condition
			//3000 rpm rev limiter
			if (predRotInt <= 1250) revLimit = true;
			else if (predRotInt <= 1500) revLimit = false;
			
			if (!injected && (getEngineAngle() >= 450.0)) { //90deg after exhaust stroke ends
				
				if (!revLimit) pulse(pulseLen); 
				injected = true; 
			}
		}
		recv(&processFrame);
    }
	return 0;
}

//ISR (TIMER0_COMPA_vect) IN injector.c

/* void initADC() {
	ADMUX |= (1 << REFS0); //Use Vcc as Reference
							//ADC0 is used as pin by default					
							//free running mode by defult
							
	ADCSRA |= 0b101; // prescaler at 32. 13 cycles per conversion. 26us per conversion
	ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADATE); //Enable ADC, start conversion, and enable autotriggering
} */


//hall effect sensor code

	
void initEngineTimer() {
	DDRD &= ~(_BV(2)); //set interrupt pin to input
	EICRA = _BV(ISC01) | _BV(ISC00); //interrupt on rising edge of INT0
	EIMSK = _BV(INT0); //enable external interrupt on pin INT0
		
	TIMSK1 = _BV(0); //enable overflow interrupt
	TCCR1B = _BV(CS12); //prescaler set at clk/256, start timer
}

float getEngineAngle() { //engine angle from 0 - 720. 0 is TDC when spark should fire
	float angle = (TCNT1 / ((float) predRotInt)) * 360.0;
	if (!powerStroke) angle += 360.0;
	angle += SENSOR_ATDC;
	angle = fmod(angle, 720.0);
	return angle;
}

ISR(INT0_vect) { //INT0 rising edge interrupt
	uint16_t newInt = TCNT1;
	if (engineRunning) {
		if (engineTiming) dRotInt = rotInt - newInt; //if the engine hasn't just started running
		rotInt = newInt;
		predRotInt = rotInt + dRotInt;
		engineTiming = true;
	}
	TCNT1 = 0; //reset engine timer
	engineRunning = true; //the engine is running
	if (powerStroke) injected = false;
	powerStroke = !powerStroke; //invert whether the next tdc is a power stroke or not
}

ISR(TIMER1_OVF_vect) {
	engineRunning = false; //if the sensor hasn't triggered by now (~1second) the engine has stopped
	engineTiming = false;
	rotInt = 0;
	dRotInt = 0;
	predRotInt = 0;
}

void processFrame() {
	switch (rxBuf[0]) {
		case HDR_FUEL:
			if (rxNew != 2) return;
			pulseLen = rxBuf[1];
			break;
		case HDR_GETDATA:
			if (rxNew != 1)  return;
			sendEngineStatus(pulseLen, engineRunning, revLimit, rotInt);
			break;
		case HDR_PULSE:
			if (rxNew != 1) return;
			pulse(pulseLen);
			break;
	}
}