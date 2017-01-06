#ifndef SERIAL_H
#define SERIAL_H

#define RX_BUFLEN 16
#define FLG_ESC 0xFD
#define FLG_ETX 0xFE
#define FLG_STX 0xFF

#define HDR_FUEL 0x00
#define HDR_GETDATA 0x01
#define HDR_PULSE 0x02

#include <avr/io.h>          // register definitions
#include "uart.h"       
#include <util/delay.h>      // delay functions
#include <stdbool.h>    

uint8_t rxNew;
uint8_t inFrame;
uint8_t inEsc;
uint8_t rxBuf[RX_BUFLEN];

void initSerial();
void recv();

uint8_t bufferFull();

uint8_t bufferEmpty();

uint8_t bufferNext();

void bufferPush(uint8_t in);

void bufferTrash();
void escapeAndSend(uint8_t *data, uint8_t len);
void sendEngineStatus(uint8_t pulseLen, bool engineRunning, bool revLimit, uint16_t rotInt);


#endif