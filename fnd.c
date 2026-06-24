#include "fnd.h"

#if FND_CONFIG_ANODE
static const uint8_t fnd_font[] = { 0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xD8, 0x80, 0x98, 0x7F, 0xFF };
#else
static const uint8_t fnd_font[] = { 0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x27, 0x7F, 0x67, 0x80, 0x00 };
#endif

fnd_t fnd1 = {
	.data_ddr = &DDRC,
	.data_port = &PORTC,
	.digit_ddr = &DDRA,
	.digit_port = &PORTA,
	.digit_pins = {3, 4, 5, 6},
	.display_buffer = {0xFF, 0xFF, 0xFF, 0xFF}
};

fnd_t fnd2 = {
	.data_ddr = &DDRF,
	.data_port = &PORTF,
	.digit_ddr = &DDRG,
	.digit_port = &PORTG,
	.digit_pins = {0, 1, 2, 3},
	.display_buffer = {0xFF, 0xFF, 0xFF, 0xFF}
};

// FND1, FND2 초기화
void init_all_fnd(void)
{
	*(fnd1.data_ddr) = 0xFF; // 출력 설정
	for(int i = 0; i < FND_DIGIT_MAX; i++) //  자릿수 핀 출력 설정
	{
		*(fnd1.digit_ddr) |= (1 << fnd1.digit_pins[i]);
	}
	
	*(fnd2.data_ddr) = 0xFF;
	for(int i = 0; i < FND_DIGIT_MAX; i++)
	{
		*(fnd2.digit_ddr) |= (1 << fnd2.digit_pins[i]);
	}
	
	fnd_clear_buffer(&fnd1);
	fnd_clear_buffer(&fnd2);
}

// 전체 소등
void fnd_clear_buffer(fnd_t *fnd)
{
	for(int i = 0; i < FND_DIGIT_MAX; i++) {
		#if FND_CONFIG_ANODE
		fnd->display_buffer[i] = 0xFF;
		#else
		fnd->display_buffer[i] = 0x00;
		#endif
	}
}

// 4자리 숫자를 각 자릿수별(1000, 100, 10, 1)로 쪼개어 버퍼에 할당
void fnd_set_number(fnd_t *fnd, uint16_t number)
{
	fnd->display_buffer[3] = fnd_font[number % 10];
	fnd->display_buffer[2] = fnd_font[(number / 10) % 10];
	fnd->display_buffer[1] = fnd_font[(number / 100) % 10];
	fnd->display_buffer[0] = fnd_font[(number / 1000) % 10];
}

// 특정 index 에 원하는 패턴 설정
void fnd_set_pattern(fnd_t *fnd, uint8_t digit_idx, uint8_t pattern)
{
	if (digit_idx >= FND_DIGIT_MAX) return;
	
	#if FND_CONFIG_ANODE
	fnd->display_buffer[digit_idx] = ~pattern;
	#else
	fnd->display_buffer[digit_idx] = pattern;
	#endif
}

// 주행 상태를 체크하여 FND 데이터 버퍼를 변경
void fnd_update_data(void)
{
	static uint32_t last_anim_time = 0;
	static uint8_t anim_index = 0;
	
	// 100ms 마다 애니메이션
	if (ms_count - last_anim_time >= 100)
	{
		last_anim_time = ms_count;
		anim_index = (anim_index + 1) % 5; // 4번 index 는 한 박자 쉬어간다
	}
	
	// 주행 중
	if (driving_state == STATE_DRIVING)
	{
		// 초기화
		uint8_t fnd2_buffer[4] = {0x00, 0x00, 0x00, 0x00};
		
		switch (driving_direction)
		{
			case DIR_FORWARD: // 직진: FFFF 문자 고정 출력 + FND2 상승 애니메이션 구현
			for(int i = 0; i < FND_DIGIT_MAX; i++) fnd_set_pattern(&fnd1, i, 0x71);
			
			if (anim_index == 0)      { for(int i=0; i<4; i++) fnd2_buffer[i] = 0x08; } // D 세그먼트 (하단 바)
			else if (anim_index == 1) { for(int i=0; i<4; i++) fnd2_buffer[i] = 0x40; } // G 세그먼트 (중간 바)
			else if (anim_index == 2) { for(int i=0; i<4; i++) fnd2_buffer[i] = 0x01; } // A 세그먼트 (상단 바)
			break;
			
			case DIR_BACKWARD: // 후진: bbbb 문자 고정 출력 + FND2 하강 애니메이션 구현
			for(int i = 0; i < FND_DIGIT_MAX; i++) fnd_set_pattern(&fnd1, i, 0x7C);
			
			if (anim_index == 0)      { for(int i=0; i<4; i++) fnd2_buffer[i] = 0x01; } // A 세그먼트 (상단 바)
			else if (anim_index == 1) { for(int i=0; i<4; i++) fnd2_buffer[i] = 0x40; } // G 세그먼트 (중간 바)
			else if (anim_index == 2) { for(int i=0; i<4; i++) fnd2_buffer[i] = 0x08; } // D 세그먼트 (하단 바)
			break;
			
			case DIR_LEFT: // 좌회전: LLLL 문자 고정 출력 + FND2 우->좌 이동 애니메이션
			for(int i = 0; i < FND_DIGIT_MAX; i++) { fnd_set_pattern(&fnd1, i, 0x38); }
			
			if (anim_index < 4) { fnd2_buffer[3 - anim_index] = 0x40; }
			break;
			
			case DIR_RIGHT: // 우회전: rrrr 문자 고정 출력 + FND2 좌->우 이동 애니메이션
			for(int i = 0; i < FND_DIGIT_MAX; i++) { fnd_set_pattern(&fnd1, i, 0x50); }
			
			if (anim_index < 4) { fnd2_buffer[anim_index] = 0x40; }
			break;
			
			case DIR_STOP:
			default: // 정지: 양 화면 모두 StOP 문자 고정 표출
			fnd_set_pattern(&fnd1, 0, 0x6D);
			fnd_set_pattern(&fnd1, 1, 0x78);
			fnd_set_pattern(&fnd1, 2, 0x3F);
			fnd_set_pattern(&fnd1, 3, 0x73);
			
			fnd_set_pattern(&fnd2, 0, 0x6D);
			fnd_set_pattern(&fnd2, 1, 0x78);
			fnd_set_pattern(&fnd2, 2, 0x3F);
			fnd_set_pattern(&fnd2, 3, 0x73);
			return;
		}
		
		// 애니메이션 패턴 버퍼에 업데이트
		for(int i = 0; i < FND_DIGIT_MAX; i++)
		{
			fnd_set_pattern(&fnd2, i, fnd2_buffer[i]);
		}
	}
	// 주행 종료 (결과 화면)
	else if (driving_state == STATE_RESULT)
	{
		// 2자리 숫자 조합 (FND1: 시간 + 후진 / FND2: 좌회전 + 우회전)
		uint16_t fnd1_number = (driving_result.total_time % 100) * 100
		+ (driving_result.backward_count % 100);
		uint16_t fnd2_number = (driving_result.left_count % 100) * 100
		+ (driving_result.right_count % 100);
		
		fnd_set_number(&fnd1, fnd1_number);
		fnd_set_number(&fnd2, fnd2_number);
	}
}

// FND1, FND2 에 각각 저장되어 있는 데이터 버퍼를 화면에 표출
void fnd_show_display(void)
{
	static uint8_t current_digit = 0;
	
	// 직전에 켜져 있던 자릿수만 OFF
	#if FND_CONFIG_ANODE
	*(fnd1.digit_port) &= ~(1 << fnd1.digit_pins[current_digit]);
	*(fnd2.digit_port) &= ~(1 << fnd2.digit_pins[current_digit]);
	#else
	*(fnd1.digit_port) |= (1 << fnd1.digit_pins[current_digit]);
	*(fnd2.digit_port) |= (1 << fnd2.digit_pins[current_digit]);
	#endif
	
	// 자릿수 업데이트
	current_digit = (current_digit + 1) % FND_DIGIT_MAX;
	
	// 데이터 로드
	*(fnd1.data_port) = fnd1.display_buffer[current_digit];
	*(fnd2.data_port) = fnd2.display_buffer[current_digit];
	
	// 현재 자릿수 핀의 데이터 표출
	#if FND_CONFIG_ANODE
	*(fnd1.digit_port) |= (1 << fnd1.digit_pins[current_digit]);
	*(fnd2.digit_port) |= (1 << fnd2.digit_pins[current_digit]);
	#else
	*(fnd1.digit_port) &= ~(1 << fnd1.digit_pins[current_digit]);
	*(fnd2.digit_port) &= ~(1 << fnd2.digit_pins[current_digit]);
	#endif
}
