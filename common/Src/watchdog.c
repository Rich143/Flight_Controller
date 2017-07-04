/*
 * watchdog.c
 *
 *  Created on: April 18, 2015
 *      Author: Michael Thiessen (mthiesse)
 */

#include <stm32f0xx_hal.h>
#include "watchdog.h"

#define IWDG_RELOAD 0xAAAA
#define IWDG_START 0xCCCC
#define IWDG_WRITE_EN 0x5555

// IWDG period [us] <125-32000000:125>
#define IWDG_PERIOD 500000  // 500ms
/*----------------------------------------------------------------------------
 Define  IWDG PR and RLR settings
 *----------------------------------------------------------------------------*/
#if   (IWDG_PERIOD >  16384000UL)
#define IWDG_PR             (6)
#define IWDGCLOCK (32000UL/256)
#elif (IWDG_PERIOD >   8192000UL)
#define IWDG_PR             (5)
#define IWDGCLOCK (32000UL/128)
#elif (IWDG_PERIOD >   4096000UL)
#define IWDG_PR             (4)
#define IWDGCLOCK  (32000UL/64)
#elif (IWDG_PERIOD >   2048000UL)
#define IWDG_PR             (3)
#define IWDGCLOCK  (32000UL/32)
#elif (IWDG_PERIOD >   1024000UL)
#define IWDG_PR             (2)
#define IWDGCLOCK  (32000UL/16)
#elif (IWDG_PERIOD >    512000UL)
#define IWDG_PR             (1)
#define IWDGCLOCK   (32000UL/8)
#else
#define IWDG_PR             (0)
#define IWDGCLOCK   (32000UL/4)
#endif
#define IWGDCLK  (32000UL/(0x04<<IWDG_PR))
#define IWDG_RLR (IWDG_PERIOD*IWGDCLK/1000000UL-1)

bool check_reset_by_watchdog()
{
	if (RCC->CSR & (1 << 29))                         // IWDG Reset Flag set
	{
		RCC->CSR |= (1 << 24);
		return true;
	}
	return false;
}

void watchdog_Init()
{
	IWDG->KR = IWDG_START;     // Start the watchdog
	IWDG->KR = IWDG_WRITE_EN;  // enable write to PR, RLR
	IWDG->PR = IWDG_PR;        // Init prescaler register
	IWDG->RLR = IWDG_RLR;      // Init reload register

	while (IWDG->SR)           // Wait for flags to be reset
	{
	}
	IWDG->KR = IWDG_RELOAD;    // Reload the watchdog
}

void watchdog_Feed()
{
	IWDG->KR = IWDG_RELOAD;
}

