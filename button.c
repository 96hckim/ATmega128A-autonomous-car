#include "button.h"

void init_button(void)
{
	BUTTON_DDR &= ~(1 << BUTTON_PIN_NUM);
}

button_state_t check_button_click(void)
{
	static button_state_t button_last_status = BUTTON_RELEASED;
	
	int current_state = (BUTTON_PIN_REG & (1 << BUTTON_PIN_NUM)) >> BUTTON_PIN_NUM;

	if (current_state == BUTTON_PRESSED && button_last_status == BUTTON_RELEASED)
	{
		_delay_ms(20);
		button_last_status = BUTTON_PRESSED;
		return BUTTON_RELEASED;
	}
	else if (current_state == BUTTON_RELEASED && button_last_status == BUTTON_PRESSED)
	{
		_delay_ms(20);
		button_last_status = BUTTON_RELEASED;
		return BUTTON_PRESSED;
	}
	
	return BUTTON_RELEASED;
}
