#ifndef BUTTON_H_
#define BUTTON_H_

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include <util/delay.h>

#define BUTTON_DDR       DDRG
#define BUTTON_PIN_REG   PING
#define BUTTON_PIN_NUM   4

typedef enum {
	BUTTON_RELEASED = 0,
	BUTTON_PRESSED
} button_state_t;

void init_button(void);
button_state_t check_button_click(void);

#endif /* BUTTON_H_ */
