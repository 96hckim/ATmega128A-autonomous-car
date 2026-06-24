/*
* uart1.c
*
* Created: 2026-06-22 오전 10:38:29
*  Author: kccistc
*/

#include "driving.h"
#include "uart1.h"

extern int transmit_uart0(char data, FILE *stream);

volatile uint8_t bt_data;

ISR(USART1_RX_vect)
{
	bt_data = UDR1;
	transmit_uart0(bt_data, NULL);
}

void init_uart1(void)
{
	UBRR1H = 0x00;
	UBRR1L = 207;
	
	UCSR1A |= 1 << U2X1;
	
	UCSR1B |= 1 << RXEN1 | 1 << TXEN1 | 1 << RXCIE1;
}

int transmit_uart1(char data, FILE *stream)
{
	while (!(UCSR1A & (1 << UDRE1)));
	
	UDR1 = data;
	
	return 0;
}
