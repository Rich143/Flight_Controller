#include "fc.h"

#include "freertos.h"
#include "task.h"

#include "ppm.h"
#include "pins.h"
#include "debug.h"

#define PPM_IN_PIN GPIO_PIN_0
#define PPM_IN_PORT GPIOA

#define MINIMUM_FRAME_SPACE_US 4000
#define MAXIMUM_PULSE_SPACE_US 2100 // Channel values range from 1000-2000, set this slightly higher so don't resync unnecessarily

#define PPM_QUEUE_LENGTH       10

TIM_HandleTypeDef htim5;
QueueHandle_t ppmSignalQueue;

/* TIM5 init function */
void ppmInit(void)
{

  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;
  TIM_IC_InitTypeDef sConfigIC;
  RCC_ClkInitTypeDef    clkconfig;
  uint32_t              uwTimclock, uwAPB1Prescaler = 0U;
  uint32_t              uwPrescalerValue = 0U;
  uint32_t              pFLatency;

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

  /* Compute the prescaler value to have TIM5 counter clock equal to 1MHz */
  uwPrescalerValue = (uint32_t) ((uwTimclock / 1000000U) - 1U);

  htim5.Instance = TIM5;
  htim5.Init.Prescaler = uwPrescalerValue;
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

  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  sConfigIC.ICFilter = 10;
  if (HAL_TIM_IC_ConfigChannel(&htim5, &sConfigIC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler("Failed to init timer\n");
  }

  ppmSignalQueue = xQueueCreate(PPM_QUEUE_LENGTH, sizeof(tPpmSignal));

  if (ppmSignalQueue == NULL)
  {
      Error_Handler("Failed to create ppm signal queue\n");
  }

  if(HAL_TIM_IC_Start_IT(&htim5, TIM_CHANNEL_1) != HAL_OK)
  {
      /* Starting Error */
      Error_Handler("Failed to start timer input capture\n");
  }
}

volatile uint32_t   lastCaptureUs = 0;
volatile int        ppmCurrentInputChannel = RC_CHANNEL_IN_COUNT;
volatile tPpmSignal ppmSignal;

void ppmReSync(void)
{
    ppmCurrentInputChannel = RC_CHANNEL_IN_COUNT;
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (htim->Instance == TIM5)
    {
        uint32_t captureUs = __HAL_TIM_GetCompare(&htim5, TIM_CHANNEL_1);    //read TIM5 channel 1 capture value
        uint32_t pulseLength = captureUs - lastCaptureUs;

        // Check if expecting frame space or an rc channel
        if (ppmCurrentInputChannel == RC_CHANNEL_IN_COUNT)
        {
            // This should be frame space
            if (pulseLength < MINIMUM_FRAME_SPACE_US)
            {
                ppmReSync();
            }
            else
            {
                // It was frame space, next interval will be channel 0
                ppmCurrentInputChannel = 0;
            }
        }
        else
        {
            // we are expecting a channel, check this
            if (pulseLength > MAXIMUM_PULSE_SPACE_US)
            {
                ppmReSync();
            }
            else
            {
                // its a good signal, record it and move on to next channel
                ppmSignal.signals[ppmCurrentInputChannel++] = pulseLength;

                if (ppmCurrentInputChannel == RC_CHANNEL_IN_COUNT)
                {
                    // We have gone through all the channels
                    // This means we received a valid ppm frame
                    // Post this to the queue
                    xQueueSendFromISR(ppmSignalQueue, (void *)&ppmSignal, &xHigherPriorityTaskWoken);
                }

            }
        }

        // record the current time
        lastCaptureUs = captureUs;
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
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

    tPpmSignal ppmSignal = {0};
    for ( ;; )
    {
        if (xQueueReceive(ppmSignalQueue, &ppmSignal, portMAX_DELAY) != pdTRUE)
        {
            DEBUG_PRINT("Failed to receive ppm signal\n");
        }

        DEBUG_PRINT("Channels - ");
        for (int i=0; i<RC_CHANNEL_IN_COUNT; i++)
        {
            DEBUG_PRINT("%d: %d ", i, ppmSignal.signals[i]);
        }
        DEBUG_PRINT("\n");
    }
}
