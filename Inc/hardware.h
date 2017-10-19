#ifndef HARDWARE_H_
#define HARDWARE_H_

#include "freertos.h"


typedef enum RGB_Colour {
    RGB_RED   = 1,
    RGB_BLUE  = 2,
    RGB_GREEN = 3,
} RGB_Colour;

void Clock_Config();
void ClockHSE_Config();
void hardware_init();
void rgbSetColour(RGB_Colour colour);

#endif /* HARDWARE_H_ */
