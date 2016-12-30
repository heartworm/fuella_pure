#ifndef INJECTOR_H
#define INJECTOR_H

#include <avr/io.h>          // register definitions
#include <avr/interrupt.h>   // interrupt vectors

uint8_t pulseReady;

void initPulser();
void pulse(int len);
uint8_t isPulseReady();

#endif