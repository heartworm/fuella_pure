#ifndef SERIAL_H
#define SERIAL_H

#define RX_BUFLEN 16
#define FLG_ESC 0xFD
#define FLG_ETX 0xFE
#define FLG_STX 0xFF

#define HDR_FUEL 0x00

#include <avr/io.h>          // register definitions
#include <uart.h>       

uint8_t rxNew;
uint8_t rxOld;
uint8_t inFrame;
uint8_t inEsc;
uint8_t rxBuf[RX_BUFLEN];

uint8_t txBuf[RX_BUFLEN];


void initUART();
void recv();
void processFrame();

uint8_t bufferFull();

uint8_t bufferEmpty();

uint8_t bufferNext();

void bufferPush(uint8_t in);

void bufferTrash();
 
uint8_t bufferPeek();


#endif