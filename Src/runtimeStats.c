#include "fc.h"

#include "freertos.h"
#include "task.h"

#include "runtimeStats.h"
#include "debug.h"

TIM_HandleTypeDef htim6;

uint32_t counterVal = 0; // store the counter value to help protect againts 16 bit overflow
uint16_t lastCounterVal = 0;

void statsTimerInit()
{
  TIM_MasterConfigTypeDef sMasterConfig;
  RCC_ClkInitTypeDef      clkconfig;
  uint32_t                uwTimclock, uwAPB1Prescaler = 0U;
  uint32_t                uwPrescalerValue = 0U;
  uint32_t                pFLatency;

  __HAL_RCC_TIM6_CLK_ENABLE();

  /* Get clock configuration */
  HAL_RCC_GetClockConfig(&clkconfig, &pFLatency);

  /* Get APB1 prescaler */
  uwAPB1Prescaler = clkconfig.APB1CLKDivider;

  /* Compute TIM5 clock */
  if (uwAPB1Prescaler == RCC_HCLK_DIV1) 
  {
    uwTimclock = HAL_RCC_GetPCLK1Freq();
  }
  else
  {
    uwTimclock = 2*HAL_RCC_GetPCLK1Freq();
  }

  /* Compute the prescaler value to have TIM5 counter clock equal to 10kHz */
  uwPrescalerValue = (uint32_t) ((uwTimclock / 10000U) - 1U);

  htim6.Instance = TIM6;
  htim6.Init.Prescaler = uwPrescalerValue;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 0xFFFF;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
      DEBUG_PRINT("Failed to init tim6\n");
      while(1);
      Error_Handler();
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
  {
      DEBUG_PRINT("Failed to config sync tim6\n");
      while(1);
      Error_Handler();
  }

  if (HAL_TIM_Base_Start(&htim6) != HAL_OK)
  {
      DEBUG_PRINT("Failed to start tim6\n");
      while(1);
      Error_Handler();
  }
}

uint32_t runTimeStatsGetTimerVal()
{
    uint64_t curCounterVal;
    uint16_t val, elapsed;

    portDISABLE_INTERRUPTS();

    val = __HAL_TIM_GET_COUNTER(&htim6);

    elapsed = val - lastCounterVal;

    counterVal += elapsed;
    /*counterVal += 10;*/
    /*DEBUG_PRINT("val %u,last %u,counter %llu\n", val, lastCounterVal, counterVal);*/

    lastCounterVal = val;
    curCounterVal = counterVal;

    portENABLE_INTERRUPTS();

    return curCounterVal;
}


