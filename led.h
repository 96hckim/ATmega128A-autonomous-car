#ifndef LED_H_
#define LED_H_

#include <avr/io.h>

#define LED_DDR		DDRA
#define LED_PORT	PORTA
#define LED_PIN		7

void init_led(void);
void led_on(void);
void led_off(void);

#endif /* LED_H_ */