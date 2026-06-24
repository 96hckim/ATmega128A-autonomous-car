/**
* @file    pwm.c
* @brief   16비트 타이머3 및 하드웨어 PWM 기반 DC 모터 제어 드라이버
* @date    2026-06-18
* @author  kccistc
*/

#include "pwm.h"

#define MOTOR_PWM_DDR            DDRB
#define MOTOR_LEFT_PWM_PIN       5   // OC1A: 왼쪽 바퀴 속도 제어 (ENA)
#define MOTOR_RIGHT_PWM_PIN      6   // OC1B: 오른쪽 바퀴 속도 제어 (ENB)

#define driving_dirECTION_DDR      DDRD
#define driving_dirECTION_PORT     PORTD
#define IN1_PIN                  4   // 왼쪽 바퀴 제어선 1
#define IN2_PIN                  5   // 왼쪽 바퀴 제어선 2
#define IN3_PIN                  6   // 오른쪽 바퀴 제어선 1
#define IN4_PIN                  7   // 오른쪽 바퀴 제어선 2

void clear_direction_pins(void)
{
	driving_dirECTION_PORT &= ~((1 << IN1_PIN) | (1 << IN2_PIN) | (1 << IN3_PIN) | (1 << IN4_PIN));
}

/**
* @brief 타이머1 하드웨어 PWM 초기화 설정
* @details 분주비 64, Fast PWM 모드 14 (TOP: ICR1)
* 비반전 모드 (Compare Match 시 Low, Bottom 시 High)
* 주기: ICR1(1023) * 4us = 약 4.096ms (약 244Hz)
*/
void init_timer1_pwm(void)
{
	// 1. 모드 14 설정 : 16비트 고속 PWM (Fast PWM, TOP = ICR1)
	TCCR1A |= (1 << WGM11);
	TCCR1B |= (1 << WGM13) | (1 << WGM12);
	
	// 2. 비반전 모드(Non-Inverting Mode) 설정
	// 비교 일치(Compare Match) 시 핀 출력 Low, 바텀(Bottom)에서 High 전환
	TCCR1A |= (1 << COM1A1) | (1 << COM1B1);
	
	// 3. 타이머 TOP 값 및 분주비(Prescaler) 설정
	ICR1 = 0x3FF;                                // TOP 값 결정 (10비트 해상도 수준)
	TCCR1B |= (1 << CS11) | (1 << CS10);        // 64 분주비 설정 (16MHz / 64 = 250kHz)
}

/**
* @brief 모터 드라이버(L298N) GPIO 방향성 및 초기 동작 제어 설정
*/
void init_motor_driver(void)
{
	// 1. 모터 PWM 제어 핀(PB5, PB6) 출력 모드 설정
	MOTOR_PWM_DDR |= (1 << MOTOR_LEFT_PWM_PIN) | (1 << MOTOR_RIGHT_PWM_PIN);
	
	// 2. 모터 방향 제어 핀(PD4 ~ PD7) 출력 모드 설정
	driving_dirECTION_DDR |= (1 << IN1_PIN) | (1 << IN2_PIN) | (1 << IN3_PIN) | (1 << IN4_PIN);
	
	// 3. 초기 모터 구동 상태 설정
	clear_direction_pins();
}

void forward(uint16_t speed)
{
	clear_direction_pins();
	driving_dirECTION_PORT |= (1 << IN1_PIN) | (1 << IN3_PIN);
	
	OCR1A = OCR1B = speed;
}

void backward(uint16_t speed)
{
	clear_direction_pins();
	driving_dirECTION_PORT |= (1 << IN2_PIN) | (1 << IN4_PIN);
	
	OCR1A = OCR1B = speed;
}

void turn_left(uint16_t speed)
{
	clear_direction_pins();
	driving_dirECTION_PORT |= (1 << IN2_PIN) | (1 << IN3_PIN);
	
	OCR1A = OCR1B = speed;
}

void turn_right(uint16_t speed)
{
	clear_direction_pins();
	driving_dirECTION_PORT |= (1 << IN1_PIN) | (1 << IN4_PIN);
	
	OCR1A = OCR1B = speed;
}

void stop(void)
{
	clear_direction_pins();
	driving_dirECTION_PORT |= (1 << IN1_PIN) | (1 << IN2_PIN) | (1 << IN3_PIN) | (1 << IN4_PIN);
	
	OCR1A = OCR1B = 0;
}
