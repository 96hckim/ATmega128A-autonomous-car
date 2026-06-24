/*
* file: main.c
*
* Created: 2026-06-15 오전 11:36:39
* Author : kccistc
*/

#define F_CPU 16000000UL

#include "driving.h"
#include "button.h"
#include "led.h"
#include "fnd.h"
#include "uart0.h"
#include "uart1.h"
#include "pwm.h"
#include "ultrasonic.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>

void init_timer0(void);
void handle_button_click(void);
void update_fnd_data(void);
void manual_mode(void);
void auto_mode(void);
void change_driving_direction(driving_direction_t new_direction, uint16_t speed);

FILE OUTPUT = FDEV_SETUP_STREAM(transmit_uart0, NULL, _FDEV_SETUP_WRITE);
volatile uint32_t ms_count = 0;

driving_mode_t current_mode = MODE_MANUAL;			// 현재 주행 모드 (수동/오토)
driving_result_t driving_result = { 0 };			// 주행 데이터 통계 구조체
driving_state_t driving_state = STATE_DRIVING;		// FND 화면 표시 상태
driving_direction_t driving_direction = DIR_STOP;	// 차량 현재 주행 방향

uint32_t debounce_start_time = 0;
uint16_t debounce_time_ms = 0;

extern volatile uint8_t bt_data;
extern volatile uint16_t ultrasonic_distances[];

void (*driving_mode_funcs[])(void) =
{
	manual_mode,
	auto_mode
};

ISR(TIMER0_OVF_vect)
{
	TCNT0 = 6;
	
	ms_count++; // 1ms 카운트
}

int main(void)
{
	init_timer0();
	init_button();
	init_led();
	init_all_fnd();
	init_uart0();
	init_uart1();
	init_timer1_pwm();
	init_motor_driver();
	init_ultrasonic();
	
	stdout = &OUTPUT;
	sei();
	
	while (1)
	{
		// 초음파 센서 트리거
		ultrasonic_scan_sensors();

		// 버튼 입력 처리
		handle_button_click();

		// 오토 모드 진입 시 LED ON
		if (current_mode == MODE_AUTO)
		{
			led_on();
		}
		else
		{
			led_off();
		}

		// 수동/오토 주행 제어
		driving_mode_funcs[current_mode]();

		// FND 데이터 업데이트 및 표출
		update_fnd_data();
	}
	
	return 0;
}

// 타이머0 초기화 설정 (64분주, 1ms 주기 오버플로우 세팅)
void init_timer0(void)
{
	TCNT0 = 6;
	TCCR0 &= ~(1 << CS02 | 1 << CS01 | 1 << CS00);
	TCCR0 |= 1 << CS02 | 0 << CS01 | 0 << CS00;
	TIMSK |= 1 << TOIE0;
}

// 버튼 클릭 이벤트 처리
void handle_button_click(void)
{
	if (check_button_click() != BUTTON_PRESSED) return;
	
	switch (driving_state)
	{
		case STATE_DRIVING:
		if (current_mode == MODE_MANUAL)
		{
			// 정지 대기 상태에서 버튼 클릭 시 -> 오토 모드 자율주행 시작
			current_mode = MODE_AUTO;
			fnd_clear_buffer(&fnd1);
			fnd_clear_buffer(&fnd2);
			
			// 주행 통계 초기화
			driving_result = (driving_result_t) { 0 };
			
			debounce_start_time = ms_count;
			debounce_time_ms = 0;
			change_driving_direction(DIR_STOP, 0);
		}
		else
		{
			// 오토 자율주행 중 버튼 클릭 시 -> 주행 종료 및 결과 화면 진입
			current_mode = MODE_MANUAL;
			change_driving_direction(DIR_STOP, 0);
			
			// 주행 종료 시점에 총 주행 시간(초) 연산
			if (driving_result.start_time != 0)
			{
				driving_result.total_time =
				(ms_count - driving_result.start_time) / 1000;
			}
			
			driving_state = STATE_RESULT; // 결과 화면 상태 전환
		}
		break;
		
		case STATE_RESULT:
		// 결과 화면에서 버튼 클릭 시 -> 전체 리셋 후 주행 대기 상태
		driving_result = (driving_result_t) { 0 };
		driving_state = STATE_DRIVING;
		break;
		
		default:
		break;
	}
}

// FND 데이터 표출
void update_fnd_data(void)
{
	static uint32_t last_update_ms = 0;
	
	if (ms_count - last_update_ms >= 1) // 1ms
	{
		last_update_ms = ms_count;
		
		fnd_update_data();  // 주행 상태에 따른 FND 출력 데이터 갱신
		fnd_show_display(); // 업데이트 된 데이터를 바탕으로 FND 표출
	}
}

// 블루투스 패킷 수신하여 수동 조종 제어
void manual_mode(void)
{
	switch(bt_data)
	{
		// 전진
		case 'F':
		case 'f':
		change_driving_direction(DIR_FORWARD, 300);
		break;
		
		// 후진
		case 'B':
		case 'b':
		change_driving_direction(DIR_BACKWARD, 300);
		break;
		
		// 좌회전
		case 'L':
		case 'l':
		change_driving_direction(DIR_LEFT, 300);
		break;
		
		// 우회전
		case 'R':
		case 'r':
		change_driving_direction(DIR_RIGHT, 300);
		break;
		
		// 정지
		case 'S':
		case 's':
		change_driving_direction(DIR_STOP, 0);
		break;
		
		default:
		break;
	}
}

// 3방향 초음파 센서 거리을 이용한 주행 방향 제어
void auto_mode(void)
{
	uint16_t left_distance = ultrasonic_distances[US_LEFT];
	uint16_t center_distance = ultrasonic_distances[US_CENTER];
	uint16_t right_distance = ultrasonic_distances[US_RIGHT];
	
	// 주행 방향 전환 후 디바운싱 시간 체크
	if (ms_count - debounce_start_time < debounce_time_ms) return;
	
	// 정면 장애물
	if (center_distance < 11)
	{
		change_driving_direction(DIR_BACKWARD, 400);
		debounce_time_ms = 280;
	}
	// 원거리 장애물 및 코너링
	else if (center_distance < 18)
	{
		if (left_distance >= right_distance)
		{
			change_driving_direction(DIR_LEFT, 440);
		}
		else
		{
			change_driving_direction(DIR_RIGHT, 440);
		}
		debounce_time_ms = 140;
	}
	// 좌측 장애물
	else if (left_distance < 11)
	{
		change_driving_direction(DIR_RIGHT, 380);
		debounce_time_ms = 100;
	}
	// 우측 장애물
	else if (right_distance < 11)
	{
		change_driving_direction(DIR_LEFT, 380);
		debounce_time_ms = 100;
	}
	// 장애물 X
	else
	{
		change_driving_direction(DIR_FORWARD, 300);
		debounce_time_ms = 0;
	}
	
	// 디바운싱 시간 최신화
	debounce_start_time = ms_count;
}

// 모터 드라이버 주행 방향 전달 및 실시간 주행 통계
void change_driving_direction(driving_direction_t new_direction, uint16_t speed)
{
	if (driving_direction == new_direction) return;
	
	driving_direction = new_direction;
	
	// 최초로 차량이 움직이기 시작할 때 타이머 측정 시작 시간 기록
	if (new_direction != DIR_STOP && driving_result.start_time == 0)
	{
		driving_result.start_time = ms_count;
	}
	
	switch(new_direction)
	{
		case DIR_FORWARD:
		forward(speed);
		driving_result.forward_count++;
		break;
		
		case DIR_BACKWARD:
		backward(speed);
		driving_result.backward_count++;
		break;
		
		case DIR_LEFT:
		turn_left(speed);
		driving_result.left_count++;
		break;
		
		case DIR_RIGHT:
		turn_right(speed);
		driving_result.right_count++;
		break;
		
		case DIR_STOP:
		default:
		stop();
		break;
	}
}
