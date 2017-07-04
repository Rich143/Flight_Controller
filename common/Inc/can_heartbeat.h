/*
 * can_heartbeat.h
 *
 *  Created on: Apr 20, 2015
 *      Author: KabooHahahein
 */

#ifndef CAN_HEARTBEAT_H_
#define CAN_HEARTBEAT_H_

#include "stm32f0xx_hal.h"
#include "stdint.h"
#include "stdbool.h"

typedef struct
{
	const uint16_t canId;
	uint32_t lastTickMs;
} CanHeartbeatData;

void canHeartBeatProcessing();
void receiveHeartBeat(CanRxMsgTypeDef *rxMessage);
void setupHeartbeatFilters();
void initHeartBeat (uint16_t thisIdBase);
bool hasHeartBeatExpired(uint16_t canIdBase);
bool hasAnyHeartBeatExpired ();

#endif /* CAN_HEARTBEAT_H_ */
