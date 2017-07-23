#include "stm32f4xx_hal.h"
#include "hardware.h"
#include "pins_common.h"
#include "pins.h"

I2C_HandleTypeDef I2cHandle;

/** System Clock Configuration
 */
void SystemClock_Config() {
    RCC_ClkInitTypeDef RCC_ClkInitStruct;
    RCC_OscInitTypeDef RCC_OscInitStruct;

    /* Enable Power Control clock */
    __HAL_RCC_PWR_CLK_ENABLE();

    /* The voltage scaling allows optimizing the power consumption when the device is
       clocked below the maximum system frequency, to update the voltage scaling value
       regarding system frequency refer to product datasheet.  */
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /* Enable HSI Oscillator and activate PLL with HSI as source */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = 0x10;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM = 16;
    RCC_OscInitStruct.PLL.PLLN = 200;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 15;
    RCC_OscInitStruct.PLL.PLLR = 7;

    if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        /*Error_Handler();*/
    }

    /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
       clocks dividers */
    RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
    {
        /*Error_Handler();*/
    }

    /* Enable appropriate peripheral clocks */
    __SYSCFG_CLK_ENABLE();

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();
}

void setup_inputs() {
    /*GPIO_InitTypeDef GPIO_InitStruct;*/
    /*GPIO_InitStruct.Mode = GPIO_MODE_INPUT;*/
    /*GPIO_InitStruct.Pull = GPIO_PULLUP;*/

    // GPIOA
    //HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // GPIOB
    //HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // GPIOC
    //HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

void setup_outputs() {
    /*GPIO_InitTypeDef GPIO_InitStruct;*/
    /*GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;*/
    /*GPIO_InitStruct.Pull = GPIO_NOPULL;*/

    // GPIOA
    //HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // GPIOB
    //HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // GPIOC
    //HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

void setup_adcs() {
}

/*void setup_I2C() {*/
    /*I2cHandle.Instance = I2Cx;*/

    /*I2cHandle.Init.Timing          = I2C_TIMING;*/
    /*I2cHandle.Init.OwnAddress1     = I2C_ADDRESS;*/
    /*I2cHandle.Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;*/
    /*I2cHandle.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;*/
    /*I2cHandle.Init.OwnAddress2     = 0xFF;*/
    /*I2cHandle.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;*/
    /*I2cHandle.Init.NoStretchMode   = I2C_NOSTRETCH_DISABLE;*/

    /*HAL_I2C_Init(&I2cHandle);*/

    /*[> Enable the Analog I2C Filter <]*/
    /*HAL_I2CEx_ConfigAnalogFilter(&I2cHandle,I2C_ANALOGFILTER_ENABLE);*/
/*}*/

/**
  * @brief I2C MSP Initialization
  *        This function configures the hardware resources used in this example:
  *           - Peripheral's clock enable
  *           - Peripheral's GPIO Configuration
  *           - DMA configuration for transmission request by peripheral
  *           - NVIC configuration for DMA interrupt request enable
  * @param hi2c: I2C handle pointer
  * @retval None
  */
/*void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c)*/
/*{*/
  /*GPIO_InitTypeDef  GPIO_InitStruct;*/
  /*RCC_PeriphCLKInitTypeDef  RCC_PeriphCLKInitStruct;*/

  /*[>##-1- Configure the I2C clock source. The clock is derived from the SYSCLK #<]*/
  /*RCC_PeriphCLKInitStruct.PeriphClockSelection = RCC_PERIPHCLK_I2Cx;*/
  /*RCC_PeriphCLKInitStruct.I2c1ClockSelection = RCC_I2CxCLKSOURCE_SYSCLK;*/
  /*HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphCLKInitStruct);*/

  /*[>##-2- Enable peripherals and GPIO Clocks #################################<]*/
  /*[> Enable GPIO TX/RX clock <]*/
  /*I2Cx_SCL_GPIO_CLK_ENABLE();*/
  /*I2Cx_SDA_GPIO_CLK_ENABLE();*/
  /*[> Enable I2Cx clock <]*/
  /*I2Cx_CLK_ENABLE();*/

  /*[>##-3- Configure peripheral GPIO ##########################################<]*/
  /*[> I2C TX GPIO pin configuration  <]*/
  /*GPIO_InitStruct.Pin       = I2Cx_SCL_PIN;*/
  /*GPIO_InitStruct.Mode      = GPIO_MODE_AF_OD;*/
  /*GPIO_InitStruct.Pull      = GPIO_PULLUP;*/
  /*GPIO_InitStruct.Speed     = GPIO_SPEED_HIGH;*/
  /*GPIO_InitStruct.Alternate = I2Cx_SCL_SDA_AF;*/

  /*HAL_GPIO_Init(I2Cx_SCL_GPIO_PORT, &GPIO_InitStruct);*/

  /*[> I2C RX GPIO pin configuration  <]*/
  /*GPIO_InitStruct.Pin       = I2Cx_SDA_PIN;*/
  /*GPIO_InitStruct.Alternate = I2Cx_SCL_SDA_AF;*/

  /*HAL_GPIO_Init(I2Cx_SDA_GPIO_PORT, &GPIO_InitStruct);*/

/*}*/

/**
  * @brief I2C MSP De-Initialization
  *        This function frees the hardware resources used in this example:
  *          - Disable the Peripheral's clock
  *          - Revert GPIO, DMA and NVIC configuration to their default state
  * @param hi2c: I2C handle pointer
  * @retval None
  */
/*void HAL_I2C_MspDeInit(I2C_HandleTypeDef *hi2c)*/
/*{*/

  /*[>##-1- Reset peripherals ##################################################<]*/
  /*I2Cx_FORCE_RESET();*/
  /*I2Cx_RELEASE_RESET();*/

  /*[>##-2- Disable peripherals and GPIO Clocks #################################<]*/
  /*[> Configure I2C Tx as alternate function  <]*/
  /*HAL_GPIO_DeInit(I2Cx_SCL_GPIO_PORT, I2Cx_SCL_PIN);*/
  /*[> Configure I2C Rx as alternate function  <]*/
  /*HAL_GPIO_DeInit(I2Cx_SDA_GPIO_PORT, I2Cx_SDA_PIN);*/
/*}*/

void MX_GPIO_Init() {
        /* Initialize Green LED GPIO on Discovery board */
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.Pin = LED_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
    HAL_GPIO_Init(LED_PORT, &GPIO_InitStruct);

    setup_inputs();
    setup_outputs();
    setup_adcs();
    /*setup_I2C();*/
}
