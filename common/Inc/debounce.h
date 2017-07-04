/*
 * debounce.h
 *
 *  Created on: Apr 22, 2015
 *      Author: arruti
 */

#ifndef DEBOUNCE_H_
#define DEBOUNCE_H_

#include <stdbool.h>
#include <stdint.h>

typedef struct
{
	bool last_press_state;
	uint32_t count;
	uint32_t last_systicks;
} button_debounce;

bool debounce_check_button_with_threshold(button_debounce* debounce, bool current_state, uint32_t threshold_count, uint32_t sample_period);

bool debounce_check_button(button_debounce* debounce, bool current_state);

#define DEBOUNCE_THRESHOLD_COUNT (20)
#define DEBOUNCE_SAMPLE_PERIOD_MS (1)

#endif /* DEBOUNCE_H_ */
