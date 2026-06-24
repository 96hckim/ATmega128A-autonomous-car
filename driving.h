/*
* driving.h
*
* Created: 2026-06-19 오전 9:47:25
*  Author: kccistc
*/


#ifndef DRIVING_H_
#define DRIVING_H_

#include <stdint.h>

// 주행 제어 모드
typedef enum {
	MODE_MANUAL = 0,
	MODE_AUTO
} driving_mode_t;

typedef struct {
	uint32_t start_time;		// 시작 시점 (ms)
	uint32_t total_time;		// 최종 총 주행 시간 (초)
	uint16_t forward_count;		// 전진 횟수
	uint16_t backward_count;	// 후진 횟수
	uint16_t left_count;		// 좌회전 횟수
	uint16_t right_count;		// 우회전 횟수
} driving_result_t;

typedef enum {
	DIR_STOP = 0,
	DIR_FORWARD,
	DIR_BACKWARD,
	DIR_LEFT,
	DIR_RIGHT
} driving_direction_t;

typedef enum {
	STATE_DRIVING = 0,
	STATE_RESULT
} driving_state_t;

#endif /* DRIVING_H_ */
