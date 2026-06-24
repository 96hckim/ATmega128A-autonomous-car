/**
* @file    ultrasonic.h
* @brief   ATmega128A 타이머3 및 외부 INT5,6,7 기반 3채널 초음파 센서 드라이버
* @date    2026-06-22
* @author  kccistc
*/

#ifndef ULTRASONIC_H_
#define ULTRASONIC_H_

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include "driving.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

// 초음파 센서 방향 채널
typedef enum {
	US_LEFT = 0,
	US_CENTER,
	US_RIGHT,
	US_MAX
} ultrasonic_direction_t;

void init_ultrasonic(void);
void ultrasonic_send_trigger(ultrasonic_direction_t direction);
void ultrasonic_scan_sensors(void);

#endif /* ULTRASONIC_H_ */
