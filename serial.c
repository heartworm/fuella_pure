#include "serial.h"

uint8_t rxNew = 0;
uint8_t inFrame = 0;
uint8_t inEsc = 0;

void initSerial() {
	uart_init(UART_BAUD_SELECT(38400, 16000000UL));
}

void recv(void (*processFrame)()) {
//void recv() {
	unsigned int statusIn = uart_getc();
	unsigned int status = statusIn & 0xFF00;
	uint8_t in = statusIn & 0x00FF;
	
/* 	if (inFrame) PORTB |= _BV(5);
	else PORTB &= ~_BV(5); */
	if (status == 0) {
		if (bufferFull()) bufferTrash();
		
		if (!inFrame) inEsc = 0;	
		if (!inEsc) {
			if (in == FLG_ESC && inFrame) {
				inEsc = 1;
			} else if (in == FLG_STX) {
				bufferTrash();
				inFrame = 1;
			} else if (in == FLG_ETX) {
				if (inFrame) processFrame();
				inFrame = 0;
			} else {
				if (inFrame) bufferPush(in);
			}
		} else {
			inEsc = 0;
			if (inFrame) bufferPush(in);
		}
	} else if (status != UART_NO_DATA) {
		bufferTrash();
	}
}

uint8_t bufferFull() {
	return rxNew >= RX_BUFLEN;
}

uint8_t bufferEmpty() {
	return rxNew <= 0;
}

uint8_t bufferNext() {
	return 0;
}

void bufferPush(uint8_t in) {
	rxBuf[rxNew++] = in;
} 

void bufferTrash() {
	rxNew = 0;
	inEsc = 0;
	inFrame = 0;
}