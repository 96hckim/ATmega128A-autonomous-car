/**
* @file    ultrasonic.c
* @brief   타이머3 공유 및 외부 인터럽트 개별 트리거링 구현
*/

#include "ultrasonic.h"

extern volatile uint32_t ms_count;

// 실시간 초음파 거리 저장 전역 변수 (왼쪽, 중앙, 오른쪽)
volatile uint16_t ultrasonic_distances[US_MAX] = { 400, 400, 400 };

// [왼쪽 센서] INT5 - PE5
ISR(INT5_vect)
{
	if (PINE & (1 << 5))
	{
		TCNT3 = 0;
	}
	else
	{
		uint32_t duration_us = (uint32_t)TCNT3 * 64;
		ultrasonic_distances[US_LEFT] = duration_us / 58;
	}
}

// [중앙 센서] INT6 - PE6
ISR(INT6_vect)
{
	if (PINE & (1 << 6))
	{
		TCNT3 = 0;
	}
	else
	{
		uint32_t duration_us = (uint32_t)TCNT3 * 64;
		ultrasonic_distances[US_CENTER] = duration_us / 58;
	}
}

// [오른쪽 센서] INT7 - PE7
ISR(INT7_vect)
{
	if (PINE & (1 << 7))
	{
		TCNT3 = 0;
	}
	else
	{
		uint32_t duration_us = (uint32_t)TCNT3 * 64;
		ultrasonic_distances[US_RIGHT] = duration_us / 58;
	}
}

void init_ultrasonic(void)
{
	// 1. Trigger 핀 출력 설정 (PA0, PA1, PA2)
	DDRA |= (1 << 0) | (1 << 1) | (1 << 2);
	PORTA &= ~((1 << 0) | (1 << 1) | (1 << 2)); // 초기 상태 Low
	
	// 2. Echo 핀 입력 설정 (PE5, PE6, PE7)
	DDRE &= ~((1 << 5) | (1 << 6) | (1 << 7));
	
	// 3. 외부 인터럽트 설정 (INT5,6,7 모두 변할 때마다(Any Change) 인터럽트 발생)
	// EICRB 레지스터의 ISC50, ISC60, ISC70 비트를 1로 셋업
	EICRB |= (1 << ISC50) | (1 << ISC60) | (1 << ISC70);
	EICRB &= ~((1 << ISC51) | (1 << ISC61) | (1 << ISC71));
	
	// 외부 인터럽트 마스크 활성화
	EIMSK |= (1 << INT5) | (1 << INT6) | (1 << INT7);
	
	// 1024 분주비 설정 (16MHz / 1024 = 15,625Hz -> 1클록당 64us)
	TCCR3A = 0x00;
	TCCR3B |= (1 << CS32) | (1 << CS30);
}

// 15us 트리거 펄스 생성
void ultrasonic_send_trigger(ultrasonic_direction_t direction)
{
	PORTA &= ~(1 << direction);
	_delay_us(1);
	
	PORTA |= (1 << direction);
	_delay_us(15);
	
	PORTA &= ~(1 << direction);
}

// 3개의 센서를 순차적으로 스캔
void ultrasonic_scan_sensors(void)
{
	static uint32_t last_ms = 0;
	static ultrasonic_direction_t current_direction = US_LEFT;
	
	// 센서 간 신호 간섭 방지를 위해 분할 구동
	if (ms_count - last_ms >= 20)
	{
		last_ms = ms_count;
		
		ultrasonic_send_trigger(current_direction);
		
		current_direction = (current_direction + 1) % US_MAX;
	}
}
