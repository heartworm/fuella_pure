#include "lcd.h"

char* top;
char* bot;

void pulse_e() { //commit data on the parallel pins to the LCDs memory
	PORTD |= LCD_E;
	PORTD &= ~(LCD_E);
}

void writeDisplay(uint8_t data) {
	//set write mode 
	//set R/W pin LOW
	PORTD &= ~(LCD_RW);
	PORTD &= ~(LCD_E);
	//send MS 4 bits
	PORTB &= 0xE2;
	PORTB |= (((data >> 4) & 0x01) | ((data >> 3) & 0x1C)) & 0x1D;
	//confirm
	pulse_e();
	//send LS 4 bits;
	PORTB &= 0xE2;
	PORTB |= (((data) & 0x01) | ((data << 1) & 0x1C)) & 0x1D;
	//confirm
	pulse_e();
	_delay_ms(WRITE_DELAY); //delay is very large
}

void sendData(uint8_t data) {
	PORTD |= LCD_RS;
	writeDisplay(data);
	PORTD &= ~(LCD_RS);
}

void sendCommand(uint8_t data) {
	PORTD &= ~(LCD_RS);
	writeDisplay(data);
}

void sendStr(char* str) {
	while (*str != 0x00) {
		sendData(*str++);
	}
}

void lcdInit() {
	DDRB |= 1 << 4 | 1 << 3 | 1 << 2 | 1 << 0;
	DDRD |= 1 << 7 | 1 << 6 | 1 << 5;
	
	//enable 4 pin mode
	PORTB &= 0xE2;
	PORTB |= 0x4;
	pulse_e();
	_delay_ms(50);
	
	top = calloc(17, 1);
	bot = calloc(17, 1);
}

void lcdDefaults() {
	sendCommand(0b00001100);
	sendCommand(0b00101000);
	lcdClear();
}

void lcdClear() {
	sendCommand(0b00000001);
	memset(top, ' ', 16);
	memset(bot, ' ', 16);
}

void lcdDraw() {
	sendCommand(0b00000010);
	sendStr(top);
	sendCommand(0b11000000);
	sendStr(bot);
}

void lcdPutString(uint8_t c, uint8_t r, char* str) {
	if (!(c < 16 && r < 2)) return;
	char* row = r == 0 ? top : bot;
	
	row += c;
	while ((*str != 0x00) && (*row != 0x00)) {
		*(row++) = *(str++);
	}
}