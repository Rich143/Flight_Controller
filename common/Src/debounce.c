/*
 * debounce.c
 *
 *  Created on: Apr 22, 2015
 *      Author: arruti
 */

#include "stm32f0xx_hal.h"
#include <stdbool.h>
#include "debounce.h"

bool debounce_check_button_with_threshold(button_debounce* debounce, bool current_state, uint32_t threshold_count, uint32_t sample_period)
{
	uint32_t current_systicks = HAL_GetTick();
	if(current_systicks < debounce->last_systicks + sample_period)
	{
		return false;
	}
	debounce->last_systicks = current_systicks;

	if(debounce->last_press_state == current_state)
	{
		debounce->count = 0;
	}
	else
	{
		debounce->count++;
		if (debounce->count >= threshold_count)
		{
			debounce->count = 0;
			debounce->last_press_state = current_state;
			return true;
		}
	}
	return false;
}

bool debounce_check_button(button_debounce* debounce, bool current_state)
{
	return debounce_check_button_with_threshold(debounce, current_state, DEBOUNCE_THRESHOLD_COUNT, DEBOUNCE_SAMPLE_PERIOD_MS);
}

