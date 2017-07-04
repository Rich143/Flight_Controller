/*
 * can.h
 *
 *  Created on: Mar 29, 2015
 *      Author: KabooHahahein
 */

#ifndef CAN_H_
#define CAN_H_

#include "stm32f0xx_hal.h"
#include "stdbool.h"
#include "can_comm.h"

void setupCanFilters();         // You must implement this in can_data.c

#define NUM_ID_BIT_SHIFT        5

#define CAN_BOARDADDR_LEN       0x100
#define CAN_HBADDR_OFFSET       1

#define CAN_ID_BMS_BASE         0x100

#define CAN_ID_VCU_BASE         0x200

#define CAN_ID_DCU_BASE         0x300

#define CAN_ID_PDB_BASE         0x400

#define CAN_ID_DAU_BASE         0x500

#define CAN_ID_MAX              0x7FF      // 11 bits is ID max
#define CAN_NUM_ADDR_PER_FILTER 4

void addCanFilter(uint16_t addresses[CAN_NUM_ADDR_PER_FILTER]);
void canInit(const uint16_t thisIdBase);
bool sendCanMessage(const uint16_t id, const uint8_t *data, const uint8_t length);
bool sendCanMessageTimeoutMs(const uint16_t id, const uint8_t *data,
                             const uint8_t length, const uint32_t timeout);

#endif /* CAN_H_ */
