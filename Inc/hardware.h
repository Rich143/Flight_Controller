#ifndef HARDWARE_H_
#define HARDWARE_H_

#include "pins.h"

void Clock_Config();
void hardware_init();

extern I2C_HandleTypeDef I2cHandle;

#endif /* HARDWARE_H_ */
