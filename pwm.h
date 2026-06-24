/*
* pwm.h
*
* Created: 2026-06-18 오후 2:22:34
*  Author: kccistc
*/

#ifndef PWM_H_
#define PWM_H_

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>

void clear_direction_pins(void);
void init_timer1_pwm(void);
void init_motor_driver(void);
void forward(uint16_t speed);
void backward(uint16_t speed);
void turn_left(uint16_t speed);
void turn_right(uint16_t speed);
void stop(void);

#endif /* PWM_H_ */
