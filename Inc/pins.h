#ifndef PINS_H_
#define PINS_H_

#ifdef FC
#define LED_PIN GPIO_PIN_12
#define LED_PORT GPIOB
#elif defined(NUCLEO)
#define LED_PIN GPIO_PIN_5
#define LED_PORT GPIOA
#endif

#endif
