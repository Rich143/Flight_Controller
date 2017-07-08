#ifndef HARDWARE_H_
#define HARDWARE_H_

#include "pins.h"

void SystemClock_Config();
void MX_CAN_Init();
void MX_GPIO_Init();

#define LED_PIN GPIO_PIN_5
#define LED_PORT GPIOA

extern I2C_HandleTypeDef I2cHandle;
#define I2Cx I2C1
#define RCC_PERIPHCLK_I2Cx              RCC_PERIPHCLK_I2C1
#define RCC_I2CxCLKSOURCE_SYSCLK        RCC_I2C1CLKSOURCE_SYSCLK
#define I2Cx_CLK_ENABLE()               __HAL_RCC_I2C1_CLK_ENABLE()
#define I2Cx_SDA_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()
#define I2Cx_SCL_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()

#define I2Cx_FORCE_RESET()              __HAL_RCC_I2C1_FORCE_RESET()
#define I2Cx_RELEASE_RESET()            __HAL_RCC_I2C1_RELEASE_RESET()

/* Definition for I2Cx Pins */
#define I2Cx_SCL_PIN                    GPIO_PIN_6
#define I2Cx_SCL_GPIO_PORT              GPIOB
#define I2Cx_SDA_PIN                    GPIO_PIN_7
#define I2Cx_SDA_GPIO_PORT              GPIOB
#define I2Cx_SCL_SDA_AF                 GPIO_AF1_I2C1

#define I2C_TIMING                      0xB042C3C7 // 10 kHz, see reference manual RM0091 26.4.10
#define I2C_ADDRESS        0x30F // Probably not needed

#endif /* HARDWARE_H_ */
