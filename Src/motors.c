#include "fc.h"

#include "freertos.h"
#include "task.h"

#include "motors.h"
#include "debug.h"
#include "rc.h"

#define MOTOR_1_PIN GPIO_PIN_11
#define MOTOR_2_PIN GPIO_PIN_10
#define MOTOR_3_PIN GPIO_PIN_9
#define MOTOR_4_PIN GPIO_PIN_8
#define MOTOR_PORT GPIOA

#define MOTOR_OUTPUT_FREQUENCY 400
#define MOTOR_OUTPUT_PERIOD (1000000/MOTOR_OUTPUT_FREQUENCY)


TIM_HandleTypeDef htim1;

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* TIM1 init function */
void motorsInit(void)
{

  TIM_MasterConfigTypeDef sMasterConfig;
  TIM_OC_InitTypeDef sConfigOC;
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig;
  RCC_ClkInitTypeDef    clkconfig;
  uint32_t              uwTimclock, uwAPB2Prescaler = 0U;
  uint32_t              uwPrescalerValue = 0U;
  uint32_t              pFLatency;

  /* Get clock configuration */
  HAL_RCC_GetClockConfig(&clkconfig, &pFLatency);

  /* Get APB2 prescaler */
  uwAPB2Prescaler = clkconfig.APB2CLKDivider;

  /* Compute TIM1 clock */
  if (uwAPB2Prescaler == RCC_HCLK_DIV1)
  {
    uwTimclock = HAL_RCC_GetPCLK2Freq();
  }
  else
  {
    uwTimclock = 2*HAL_RCC_GetPCLK2Freq();
  }

  /* Compute the prescaler value to have TIM1 counter clock equal to 1MHz */
  uwPrescalerValue = (uint32_t) ((uwTimclock / 1000000U) - 1U);

  htim1.Instance = TIM1;
  htim1.Init.Prescaler = uwPrescalerValue;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = MOTOR_OUTPUT_PERIOD;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  if (HAL_TIM_OC_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }

  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = MOTOR_LOW_VAL_US;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_OC_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }

  if (HAL_TIM_OC_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }

  if (HAL_TIM_OC_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }

  if (HAL_TIM_OC_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }

  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }

  HAL_TIM_MspPostInit(&htim1);

}

/**
 * @brief Initialize pwm generation for all 4 motors, and set it to min val (motors off)
 *
 * @return Status
 */
FC_Status motorsStart()
{
  FC_Status rc = FC_OK;

  // Set all the outputs to low, should already be done but just in case
  __HAL_TIM_SET_COMPARE(&htim1, MOTOR_FRONT_LEFT, MOTOR_LOW_VAL_US);
  __HAL_TIM_SET_COMPARE(&htim1, MOTOR_FRONT_RIGHT, MOTOR_LOW_VAL_US);
  __HAL_TIM_SET_COMPARE(&htim1, MOTOR_BACK_LEFT, MOTOR_LOW_VAL_US);
  __HAL_TIM_SET_COMPARE(&htim1, MOTOR_BACK_RIGHT, MOTOR_LOW_VAL_US);

  if (HAL_TIM_PWM_Start(&htim1, MOTOR_FRONT_LEFT) != HAL_OK)
  {
      /* PWM Generation Error */
      DEBUG_PRINT("Failed to start motor 1\n");
      rc = FC_ERROR;
  }
  if (HAL_TIM_PWM_Start(&htim1, MOTOR_FRONT_RIGHT) != HAL_OK)
  {
      /* PWM Generation Error */
      DEBUG_PRINT("Failed to start motor 1\n");
      rc = FC_ERROR;
  }
  if (HAL_TIM_PWM_Start(&htim1, MOTOR_BACK_LEFT) != HAL_OK)
  {
      /* PWM Generation Error */
      DEBUG_PRINT("Failed to start motor 1\n");
      rc = FC_ERROR;
  }
  /* Start channel 4 */
  if (HAL_TIM_PWM_Start(&htim1, MOTOR_BACK_RIGHT) != HAL_OK)
  {
      /* PWM Generation Error */
      DEBUG_PRINT("Failed to start motor 1\n");
      rc = FC_ERROR;
  }

  return rc;
}

FC_Status motorsDeinit()
{
    if (HAL_TIM_PWM_Stop(&htim1, MOTOR_FRONT_LEFT) != HAL_OK)
    {
        DEBUG_PRINT("Failed to de init motors\n");
        return FC_ERROR;
    }
    if (HAL_TIM_PWM_Stop(&htim1, MOTOR_FRONT_RIGHT) != HAL_OK)
    {
        DEBUG_PRINT("Failed to de init motors\n");
        return FC_ERROR;
    }
    if (HAL_TIM_PWM_Stop(&htim1, MOTOR_BACK_LEFT) != HAL_OK)
    {
        DEBUG_PRINT("Failed to de init motors\n");
        return FC_ERROR;
    }
    if (HAL_TIM_PWM_Stop(&htim1, MOTOR_BACK_RIGHT) != HAL_OK)
    {
        DEBUG_PRINT("Failed to de init motors\n");
        return FC_ERROR;
    }

    return FC_OK;
}

FC_Status motorsStop()
{
  __HAL_TIM_SET_COMPARE(&htim1, MOTOR_FRONT_LEFT, MOTOR_LOW_VAL_US);
  __HAL_TIM_SET_COMPARE(&htim1, MOTOR_FRONT_RIGHT, MOTOR_LOW_VAL_US);
  __HAL_TIM_SET_COMPARE(&htim1, MOTOR_BACK_LEFT, MOTOR_LOW_VAL_US);
  __HAL_TIM_SET_COMPARE(&htim1, MOTOR_BACK_RIGHT, MOTOR_LOW_VAL_US);

  return FC_OK;
}

FC_Status setMotor(MotorNum motor, uint32_t val)
{
    FC_Status rc = FC_OK;

    if (val < MOTOR_LOW_VAL_US)
    {
        val = MOTOR_LOW_VAL_US;
        rc = FC_ERROR;
    } else if (val > MOTOR_HIGH_VAL_US)
    {
        val = MOTOR_LOW_VAL_US;
        rc = FC_ERROR;
    }

    __HAL_TIM_SET_COMPARE(&htim1, motor, val);

    return rc;
}


void vMotorsTask(void *pvParameters)
{
    DEBUG_PRINT("Init motors\n");
    motorsInit();
    DEBUG_PRINT("Inited motors\n");

    if (motorsStart() != FC_OK)
    {
        DEBUG_PRINT("Failed to start motors\n");
    }

    // TODO: Need to tune this to the length of the esc startup routine
    vTaskDelay(7000); // The esc want a low value during all of their startup routines

    uint32_t motorVal = MOTOR_LOW_VAL_US;
    for ( ;; )
    {
        if (motorVal > 1500)
        {
            motorVal = MOTOR_LOW_VAL_US;
        }

        setMotor(MOTOR_FRONT_LEFT, motorVal);
        setMotor(MOTOR_FRONT_RIGHT, motorVal);
        setMotor(MOTOR_BACK_LEFT, motorVal);
        setMotor(MOTOR_BACK_RIGHT, motorVal);

        motorVal += 10;

        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}

void HAL_TIM_MspPostInit(TIM_HandleTypeDef* htim)
{

  GPIO_InitTypeDef GPIO_InitStruct;
  if(htim->Instance==TIM1)
  {
  /* USER CODE BEGIN TIM1_MspPostInit 0 */

  /* USER CODE END TIM1_MspPostInit 0 */

    /**TIM1 GPIO Configuration
    PA8     ------> TIM1_CH1
    PA9     ------> TIM1_CH2
    PA10     ------> TIM1_CH3
    PA11     ------> TIM1_CH4
    */
    GPIO_InitStruct.Pin = MOTOR_1_PIN|MOTOR_2_PIN|MOTOR_3_PIN|MOTOR_4_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF1_TIM1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN TIM1_MspPostInit 1 */

  /* USER CODE END TIM1_MspPostInit 1 */
  }

}

void HAL_TIM_OC_MspDeInit(TIM_HandleTypeDef* htim_oc)
{

  if(htim_oc->Instance==TIM1)
  {
  /* USER CODE BEGIN TIM1_MspDeInit 0 */

  /* USER CODE END TIM1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_TIM1_CLK_DISABLE();
  }
  /* USER CODE BEGIN TIM1_MspDeInit 1 */

  /* USER CODE END TIM1_MspDeInit 1 */

}

void HAL_TIM_OC_MspInit(TIM_HandleTypeDef* htim_oc)
{

  if(htim_oc->Instance==TIM1)
  {
  /* USER CODE BEGIN TIM1_MspInit 0 */

  /* USER CODE END TIM1_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_TIM1_CLK_ENABLE();
  /* USER CODE BEGIN TIM1_MspInit 1 */

  /* USER CODE END TIM1_MspInit 1 */
  }

}
