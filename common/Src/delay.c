/*
 * delay.c
 *
 *  Created on: Feb 17, 2015
 *      Author: David Sami
 */

#include <stm32f0xx_hal.h>
#include <stm32f0xx_hal_tim.h>
#include "stdbool.h"

static TIM_HandleTypeDef asyncTimer =
{
	.Instance = TIM3,
	.Init = {
		.Prescaler = 47999,
		.CounterMode = TIM_COUNTERMODE_UP,
		.Period = 0,
		.ClockDivision = TIM_CLOCKDIVISION_DIV1
	},
	.Channel = HAL_TIM_ACTIVE_CHANNEL_1,
};

#ifndef VCU
static TIM_HandleTypeDef delay_timer =
{
	.Instance = TIM2,
	.Init = {
		.Prescaler = 47,
		.CounterMode = TIM_COUNTERMODE_UP,
		.Period = 0,
		.ClockDivision = TIM_CLOCKDIVISION_DIV1
	},
	.Channel = HAL_TIM_ACTIVE_CHANNEL_1,
};
static volatile uint8_t sleep_flag = 0;

void delay_us(uint32_t time_us)
{
	__HAL_TIM_SetCounter(&delay_timer,0);
	__HAL_TIM_SetAutoreload(&delay_timer,0xffffffff);
	HAL_TIM_Base_Start(&delay_timer);
	while(TIM2->CNT < time_us);
	HAL_TIM_Base_Stop(&delay_timer);
}

void sleep_us(uint32_t time_us)
{
	__HAL_TIM_SetCounter(&delay_timer,0);
	__HAL_TIM_SetAutoreload(&delay_timer, time_us);
	sleep_flag = 1;
	__HAL_TIM_CLEAR_IT(&delay_timer, TIM_IT_UPDATE);
	HAL_TIM_Base_Start_IT(&delay_timer);
	while (sleep_flag)
	{
		__WFI();
	}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	sleep_flag = 0;
}

void TIM2_IRQHandler(void)
{
	HAL_TIM_IRQHandler(&delay_timer);
	HAL_TIM_Base_Stop_IT(&delay_timer);
}
#endif

void delay_init()
{
#ifndef VCU
	__TIM2_CLK_ENABLE();
	HAL_NVIC_SetPriority(TIM2_IRQn, 3, 0);
	HAL_NVIC_EnableIRQ(TIM2_IRQn);

	HAL_TIM_Base_Init(&delay_timer);
#endif

	__TIM3_CLK_ENABLE();
	HAL_NVIC_SetPriority(TIM3_IRQn, 2, 0);
	HAL_NVIC_EnableIRQ(TIM3_IRQn);

	HAL_TIM_Base_Init(&asyncTimer);
}

// Implement the async timer functionality with the TIM3_IRQHandler(void)
static volatile bool asyncTimerExecuting;
void startAsyncTimer(uint16_t time_ms)
{
	asyncTimerExecuting = true;
	__HAL_TIM_SetCounter(&asyncTimer, 0);
	__HAL_TIM_SetAutoreload(&asyncTimer, time_ms);
	__HAL_TIM_CLEAR_IT(&asyncTimer, TIM_IT_UPDATE);
	HAL_TIM_Base_Start_IT(&asyncTimer);
}

void stopAsyncTimer ()
{
	HAL_TIM_Base_Stop_IT(&asyncTimer);
	__HAL_TIM_CLEAR_IT(&asyncTimer, TIM_IT_UPDATE);
	asyncTimerExecuting = false;
}

bool isAsyncTimerExecuting ()
{
	return asyncTimerExecuting;
}

void TIM3_IRQHandler(void)
{
	HAL_TIM_IRQHandler(&asyncTimer);
	stopAsyncTimer();
}

