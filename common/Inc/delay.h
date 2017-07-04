/*
 * delay.h
 *
 *  Created on: Feb 18, 2015
 *      Author: arruti
 */

#ifndef DELAY_H_
#define DELAY_H_

#include <stm32f0xx_hal.h>
#include "stdbool.h"

void delay_init();
void delay_us(uint32_t time_us);
void sleep_us(uint32_t time_us);

void startAsyncTimer(uint16_t time_ms);
void stopAsyncTimer ();
bool isAsyncTimerExecuting ();

#endif /* DELAY_H_ */
