#ifndef __FC_H
#define __FC_H

#ifndef __UNIT_TEST
#include "stm32f4xx.h"
#endif

#define _BIT(x) (1<<(x))

void assertFailed(char *file, int line);

#ifndef __UNIT_TEST
#define ASSERT(expr) \
    if (expr) \
        {} \
    else \
        assertFailed(__FILE__, __LINE__)

#else
#include <stdint.h>
#include <assert.h>
#define ASSERT assert

#include <stdio.h>
#define DEBUG_PRINT(...) printf(__VA_ARGS__)
#endif

typedef enum {
    FC_OK,
    FC_ERROR
} FC_Status;

void Error_Handler();

#endif

