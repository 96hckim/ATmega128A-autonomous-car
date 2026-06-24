#include "led.h"

void init_led(void)
{
	LED_DDR |= (1 << LED_PIN);
	led_off();
}

void led_on(void)
{
	LED_PORT |= (1 << LED_PIN);
}

void led_off(void)
{
	LED_PORT &= ~(1 << LED_PIN);
}

