#include "serial.h"

uint8_t rxOld = 0;
uint8_t rxNew = 0;
uint8_t inFrame = 0;
uint8_t inEsc = 0;

void initSerial() {
	UART_BAUD_SELECT(38400. 16000000UL);
	//enable RX, and TX
	UCSR0B |= _BV(RXEN0) | _BV(TXEN0) | _BV(RXCIE0);
	//frame format, 8 data, with stop bit and no parity - default settings
}

void recv(void (*processFrame)()) {
	if (UCSR0A & _BV(RXC0)) {
		while (UCSR0A & _BV(RXC0)) {
			uint8_t in = UDR0;
			if (!(UCSR0A & (_BV(FE0) | _BV(DOR0) | _BV(UPE0)))) { //no framing, overrun or parity errors
				if (bufferFull()) bufferTrash();
				
				
				if (!inFrame) inEsc = 0;
				
				if (!inEsc) {
					if (in == FLG_ESC && inFrame) {
						inEsc = 1;
					} else if (in == FLG_STX) {
						if (inFrame) bufferTrash();
						else inFrame = 1;
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
				
				if (inEsc) PORTB |= _BV(5);
				else PORTB &= ~_BV(5);
			}
		}
	}
}

uint8_t bufferFull() {
	return ((rxNew + 1) % RX_BUFLEN == rxOld);
}

uint8_t bufferEmpty() {
	return (rxNew == rxOld);
}

uint8_t bufferNext() {
	uint8_t out = (rxBuf[rxOld]);
	rxOld = (rxOld + 1) % RX_BUFLEN;
	return out;
}

void bufferPush(uint8_t in) {
	rxBuf[rxNew] = in;
	rxNew = (rxNew + 1) % RX_BUFLEN;
} 

void bufferTrash() {
	rxNew = rxOld;
	inEsc = 0;
	inFrame = 0;
}
 
uint8_t bufferPeek() {
	return rxBuf[rxOld];
}