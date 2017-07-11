#include "stm32f4xx_hal.h"
#include "interrupt.h"
#include "sched.h"

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
  HAL_IncTick();
  osSystickHandler();
}
