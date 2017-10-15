#ifndef HARDWARE_H_
#define HARDWARE_H_

#include "pins.h"

typedef enum RGB_Colour {
    RGB_RED   = 1,
    RGB_BLUE  = 2,
    RGB_GREEN = 3,
} RGB_Colour;

void Clock_Config();
void ClockHSE_Config();
void hardware_init();
void rgbSetColour(RGB_Colour colour);

extern I2C_HandleTypeDef I2cHandle;

#endif /* HARDWARE_H_ */
