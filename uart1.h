/*
* uart1.h
*
* Created: 2026-06-22 오전 10:38:17
*  Author: kccistc
*/

#ifndef UART1_H_
#define UART1_H_

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <string.h>

void init_uart1(void);
int transmit_uart1(char data, FILE *stream);

#endif /* UART1_H_ */
