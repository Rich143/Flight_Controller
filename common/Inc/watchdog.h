/*
 * watchdog.h
 *
 *  Created on: April 18, 2015
 *      Author: Michael Thiessen (mthiesse)
 */

#ifndef WATCHDOG_H_
#define WATCHDOG_H_

#include <stm32f0xx_hal.h>
#include "stdbool.h"

bool check_reset_by_watchdog();
void watchdog_Init();
void watchdog_Feed();

#endif /* WATCHDOG_H_ */
