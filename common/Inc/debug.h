// Waterloo Hybrid 2016

#ifndef DEBUG_H
#define DEBUG_H

#include "freertos.h"
#include "queue.h"

extern QueueHandle_t printQueue;

#define PRINT_QUEUE_LENGTH        15
#define PRINT_QUEUE_STRING_SIZE   40
#define QUEUE_SEND_TIMEOUT_TICKS  10

//#define printf DONT USE PRINTF, USE DEBUG_PRINT

/** 
 * @brief Send a debug string to the uart
 * 
 * @param ... the format string and any arguments
 * 
 * This creates a string from the format and arguments, then copies it to a
 * queue for future printing over the uart by the debug task
 * This will trim any resultant string to PRINT_QUEUE_STRING_SIZE characters
 * It will silently fail if the queue is full after waiting for
 * QUEUE_SEND_TIMEOUT_TICKS
 *
 * @return Nothing
 */
#define DEBUG_PRINT(...) \
    do { \
        char buf[PRINT_QUEUE_STRING_SIZE] = {0}; \
        snprintf(buf, PRINT_QUEUE_STRING_SIZE, __VA_ARGS__); \
        xQueueSend(printQueue, buf, QUEUE_SEND_TIMEOUT_TICKS); \
    } while(0)

void vDebugTask(void *pvParameters);
void debug_init(void);
#endif
