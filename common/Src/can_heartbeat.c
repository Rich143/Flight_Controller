/*
 * can_heartbeat.c
 *
 *  Created on: Apr 20, 2015
 *      Author: KabooHahahein
 */

#include "can_heartbeat.h"
#include "can.h"

#define CAN_NETWORK_SIZE        4

#define TIMEOUT_MS              1000
#define HEARTBEAT_DELAY_MS      200

#define HEARTBEAT_CHECK_BYTE    0x22

uint32_t lastSendTick;
uint16_t thisId;
CanHeartbeatData canHeartbeatData[CAN_NETWORK_SIZE] =
{
	{
		.canId = CAN_ID_BMS_BASE + CAN_HBADDR_OFFSET,
		.lastTickMs = 0
	},
	{
		.canId = CAN_ID_VCU_BASE + CAN_HBADDR_OFFSET,
		.lastTickMs = 0
	},
	{
		.canId = CAN_ID_DCU_BASE + CAN_HBADDR_OFFSET,
		.lastTickMs = 0
	},
	{
		.canId = CAN_ID_PDB_BASE + CAN_HBADDR_OFFSET,
		.lastTickMs = 0
	}
};

void setupHeartbeatFilters()
{
	uint16_t addresses [CAN_NUM_ADDR_PER_FILTER];
	uint8_t networkCounter = 0;
	while (networkCounter < CAN_NETWORK_SIZE)
	{
		for (uint8_t i = 0; i < CAN_NUM_ADDR_PER_FILTER; )
		{
			if (networkCounter < CAN_NETWORK_SIZE)
			{
				if (thisId != canHeartbeatData[networkCounter].canId)
				{
					addresses[i] = canHeartbeatData[networkCounter].canId;
					i++;
				}
				networkCounter ++;
			}
			else
			{
				addresses[i] = 0;
				i++;
			}
		}
		addCanFilter(addresses);
	}
}

void initHeartBeat (uint16_t thisIdBase)
{
	lastSendTick = 0;
	thisId = thisIdBase + CAN_HBADDR_OFFSET;
}

void receiveHeartBeat(CanRxMsgTypeDef *rxMessage)
{
	if (rxMessage->StdId == 0)
		return;

	uint8_t i;
	for (i = 0; i < CAN_NETWORK_SIZE; i ++)
	{
		if (rxMessage->StdId == canHeartbeatData[i].canId &&
		        rxMessage->StdId != thisId &&
		        rxMessage->Data[0] == HEARTBEAT_CHECK_BYTE)
		{
			canHeartbeatData[i].lastTickMs = HAL_GetTick();
		}
	}
}

void canHeartBeatProcessing()
{
	// Send heartbeat
	if (HAL_GetTick() > HEARTBEAT_DELAY_MS + lastSendTick)
	{
		uint8_t byte = HEARTBEAT_CHECK_BYTE;
		sendCanMessage(thisId, &byte, 1);
		lastSendTick = HAL_GetTick();
	}
}

bool hasHeartBeatExpired(uint16_t canIdBase)
{
	uint16_t canId = canIdBase + CAN_HBADDR_OFFSET;

	uint8_t i;
	bool isHeartBeatExpired = true;
	for (i = 0; i < CAN_NETWORK_SIZE; i ++)
	{
		if (canHeartbeatData[i].canId == canId &&
		        HAL_GetTick() < canHeartbeatData[i].lastTickMs + TIMEOUT_MS)
		{
			isHeartBeatExpired = false;
		}
	}
	return isHeartBeatExpired;
}

bool hasAnyHeartBeatExpired ()
{
	uint8_t i;
	bool isHeartBeatExpired = false;
	for (i = 0; i < CAN_NETWORK_SIZE; i ++)
	{
		if (canHeartbeatData[i].canId != thisId &&
		        HAL_GetTick() > canHeartbeatData[i].lastTickMs + TIMEOUT_MS)
		{
			isHeartBeatExpired = true;
		}
	}

	return isHeartBeatExpired;
}

