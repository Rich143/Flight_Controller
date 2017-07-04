#ifndef HARDWARE_H_
#define HARDWARE_H_

#include "pins.h"

void SystemClock_Config();
void MX_CAN_Init();
void MX_GPIO_Init();

#define LED_PIN GPIO_PIN_5
#define LED_PORT GPIOA

#endif /* HARDWARE_H_ */
