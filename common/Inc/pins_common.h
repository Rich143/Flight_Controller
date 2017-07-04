/*
 * pin_common.h
 *
 *  Created on: Mar 31, 2015
 *      Author: KabooHahahein
 */

#ifndef PIN_COMMON_H_
#define PIN_COMMON_H_

#include <stm32f0xx_hal.h>

// Processor inputs
#define BUTT1_PORT          GPIOC
#define BUTT1_PIN           GPIO_PIN_10
#define BUTT2_PORT          GPIOC
#define BUTT2_PIN           GPIO_PIN_11
#define BUTT3_PORT          GPIOC
#define BUTT3_PIN           GPIO_PIN_12
#define BUTT4_PORT          GPIOB
#define BUTT4_PIN           GPIO_PIN_6

// Processor outputs
#define LED1_PORT           GPIOC
#define LED1_PIN            GPIO_PIN_9
#define LED2_PORT           GPIOC
#define LED2_PIN            GPIO_PIN_8
#define LED3_PORT           GPIOC
#define LED3_PIN            GPIO_PIN_7
#define LED4_PORT           GPIOC
#define LED4_PIN            GPIO_PIN_6

// CAN
#define CAN_P_PIN           GPIO_PIN_8
#define CAN_N_PIN           GPIO_PIN_9
#define CAN_PORT            GPIOB

// UART
#ifdef USE_NUCLEO
#define UART_RX             GPIO_PIN_3
#define UART_TX             GPIO_PIN_2
#define UART_PORT           GPIOA
#else
#define UART_RX             GPIO_PIN_11
#define UART_TX             GPIO_PIN_10
#define UART_PORT           GPIOB
#endif

// Processor actions
#define IS_BUTTON_1_ON      (HAL_GPIO_ReadPin(BUTT1_PORT, BUTT1_PIN) == GPIO_PIN_SET)
#define IS_BUTTON_2_ON      (HAL_GPIO_ReadPin(BUTT2_PORT, BUTT2_PIN) == GPIO_PIN_SET)
#define IS_BUTTON_3_ON      (HAL_GPIO_ReadPin(BUTT3_PORT, BUTT3_PIN) == GPIO_PIN_SET)
#define IS_BUTTON_4_ON      (HAL_GPIO_ReadPin(BUTT4_PORT, BUTT4_PIN) == GPIO_PIN_SET)

#define LED1_ON             HAL_GPIO_WritePin(LED1_PORT, LED1_PIN, GPIO_PIN_SET)
#define LED2_ON             HAL_GPIO_WritePin(LED2_PORT, LED2_PIN, GPIO_PIN_SET)
#define LED3_ON             HAL_GPIO_WritePin(LED3_PORT, LED3_PIN, GPIO_PIN_SET)
#define LED4_ON             HAL_GPIO_WritePin(LED4_PORT, LED4_PIN, GPIO_PIN_SET)

#define LED1_OFF            HAL_GPIO_WritePin(LED1_PORT, LED1_PIN, GPIO_PIN_RESET)
#define LED2_OFF            HAL_GPIO_WritePin(LED2_PORT, LED2_PIN, GPIO_PIN_RESET)
#define LED3_OFF            HAL_GPIO_WritePin(LED3_PORT, LED3_PIN, GPIO_PIN_RESET)
#define LED4_OFF            HAL_GPIO_WritePin(LED4_PORT, LED4_PIN, GPIO_PIN_RESET)

#endif /* PIN_COMMON_H_ */
