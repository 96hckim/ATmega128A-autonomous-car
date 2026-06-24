#ifndef FND_H_
#define FND_H_

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include "driving.h"
#include <avr/io.h>

#define FND_CONFIG_ANODE    1
#define FND_DIGIT_MAX       4

typedef struct {
	volatile uint8_t *data_ddr;
	volatile uint8_t *data_port;
	volatile uint8_t *digit_ddr;
	volatile uint8_t *digit_port;
	uint8_t digit_pins[FND_DIGIT_MAX];
	uint8_t display_buffer[FND_DIGIT_MAX];
} fnd_t;

extern fnd_t fnd1;
extern fnd_t fnd2;

extern volatile uint32_t ms_count;
extern driving_result_t driving_result;
extern driving_state_t driving_state;
extern driving_direction_t driving_direction;

void init_all_fnd(void);
void fnd_set_number(fnd_t *fnd, uint16_t number);
void fnd_clear_buffer(fnd_t *fnd);
void fnd_set_pattern(fnd_t *fnd, uint8_t digit_idx, uint8_t pattern);
void fnd_update_data(void);
void fnd_show_display(void);

#endif /* FND_H_ */
