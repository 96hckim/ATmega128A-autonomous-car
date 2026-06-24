/**
* @file    uart0.h
* @brief   ATmega128A UART0 직렬 통신 드라이버 인터페이스
* @date    2026-06-16
* @author  kccistc
*/

#ifndef UART0_H_
#define UART0_H_

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <string.h>

void init_uart0(void);
int transmit_uart0(char data, FILE *stream);

#endif /* UART0_H_ */
