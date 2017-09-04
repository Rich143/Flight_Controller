/*
 * pin_common.h
 *
 *  Created on: Mar 31, 2015
 *      Author: KabooHahahein
 */

#ifndef PIN_COMMON_H_
#define PIN_COMMON_H_

#include <stm32f4xx_hal.h>

// Processor inputs
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

#endif /* PIN_COMMON_H_ */
