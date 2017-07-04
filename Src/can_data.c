// Waterloo Hybrid 2016

#include "stm32f0xx_hal.h"
#include "can.h"
#include "can_data.h"

void setupCanFilters() {
    uint16_t addresses[CAN_NUM_ADDR_PER_FILTER] = {0};
    addCanFilter(addresses);
}

void HAL_CAN_RxCpltCallback(CAN_HandleTypeDef* hcan) {
}
