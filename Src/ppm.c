#include "fc.h"

#include "freertos.h"
#include "task.h"

#include "ppm.h"
#include "pins.h"
#include "debug.h"

#define PPM_IN_PIN GPIO_PIN_0
#define PPM_IN_PORT GPIOA

TIM_HandleTypeDef htim5;

/* TIM5 init function */
static void ppmInit(void)
{

  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;
  TIM_IC_InitTypeDef sConfigIC;

  htim5.Instance = TIM5;
  /*htim5.Init.Prescaler = 100;*/
  htim5.Init.Prescaler = HAL_RCC_GetHCLKFreq()/1000000; // set the prescaler such that the input to the timer is 1 MHz
  htim5.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim5.Init.Period = 0xFFFFFFFF;
  htim5.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  if (HAL_TIM_Base_Init(&htim5) != HAL_OK)
  {
    Error_Handler("Failed to init timer\n");
  }

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim5, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler("Failed to init timer\n");
  }

  if (HAL_TIM_IC_Init(&htim5) != HAL_OK)
  {
    Error_Handler("Failed to init timer\n");
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim5, &sMasterConfig) != HAL_OK)
  {
    Error_Handler("Failed to init timer\n");
  }

  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_FALLING;
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  sConfigIC.ICFilter = 10;
  if (HAL_TIM_IC_ConfigChannel(&htim5, &sConfigIC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler("Failed to init timer\n");
  }

}

volatile uint32_t pulseLength = 0;
volatile uint32_t captureUs = 0;
volatile uint32_t lastCaptureUs = 0;
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM5)
    {
        LED1_ON
        captureUs = __HAL_TIM_GetCompare(&htim5, TIM_CHANNEL_1);    //read TIM5 channel 1 capture value
        pulseLength = captureUs - lastCaptureUs;
        lastCaptureUs = captureUs;
        /*__HAL_TIM_SetCounter(&htim5, 0);    //reset counter after input capture interrupt occurs*/
    }
}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* htim_base)
{

  GPIO_InitTypeDef GPIO_InitStruct;
  if(htim_base->Instance==TIM5)
  {
    /* Peripheral clock enable */
    __HAL_RCC_TIM5_CLK_ENABLE();
  
    /**TIM5 GPIO Configuration    
    PA0-WKUP     ------> TIM5_CH1 
    */
    GPIO_InitStruct.Pin = PPM_IN_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM5;
    HAL_GPIO_Init(PPM_IN_PORT, &GPIO_InitStruct);

    /* Peripheral interrupt init */
    HAL_NVIC_SetPriority(TIM5_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY, 0);
    HAL_NVIC_EnableIRQ(TIM5_IRQn);
  }

}

void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* htim_base)
{

  if(htim_base->Instance==TIM5)
  {
    /* Peripheral clock disable */
    __HAL_RCC_TIM5_CLK_DISABLE();
  
    /**TIM5 GPIO Configuration    
    PA0-WKUP     ------> TIM5_CH1 
    */
    HAL_GPIO_DeInit(PPM_IN_PORT, PPM_IN_PIN);

    /* Peripheral interrupt DeInit*/
    HAL_NVIC_DisableIRQ(TIM5_IRQn);

  }
}

void vRCTask(void *pvParameters)
{
    DEBUG_PRINT("Starting RC Task\n");

    ppmInit();

    DEBUG_PRINT("Initialized PPM\n");

    if(HAL_TIM_IC_Start_IT(&htim5, TIM_CHANNEL_1) != HAL_OK)
    {
        /* Starting Error */
        Error_Handler("Failed to start timer input capture\n");
    }

    /*int timerVal = 0;*/
    for ( ;; )
    {
        /*timerVal = __HAL_TIM_GetCounter(&htim5);*/
        /*DEBUG_PRINT("Tim %d,Last %lu,Cur %lu,Len %lu\n", timerVal, lastCaptureUs, captureUs, pulseLength);*/
        DEBUG_PRINT("Len %lu\n", pulseLength);
        DEBUG_PRINT("Freq %lu\n", HAL_RCC_GetHCLKFreq());
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}
