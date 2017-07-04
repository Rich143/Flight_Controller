/*
 * id_chip.c
 *
 *  Created on: Feb 17, 2015
 *      Author: David Sami
 */

#include <stm32f0xx_hal.h>
#include "id_chip.h"
#include "delay.h"
#include "string.h"

// Being lazy. Dont need one wire for anything else
#define ID_CHIP_GPIO_PORT (GPIOD)
#define ID_CHIP_GPIO_PIN (GPIO_PIN_2)
#define ID_CHIP_GPIO_PIN_NUM (2)

// Helper macros
#define ID_CHIP_GPIO_SET_INPUT() \
  do { \
    ID_CHIP_GPIO_PORT->MODER &= ~(0x3 << (2 * ID_CHIP_GPIO_PIN_NUM)); /* set to input mode */ \
  } while(0)

#define ID_CHIP_GPIO_SET_OUTPUT() \
  do { \
    ID_CHIP_GPIO_PORT->OTYPER |= (1 << ID_CHIP_GPIO_PIN_NUM); /* set to output drain type */ \
    ID_CHIP_GPIO_PORT->MODER |= (1 << (2 * ID_CHIP_GPIO_PIN_NUM)); /* set to output mode */ \
  } while(0)

#define ID_CHIP_GPIO_READ() (HAL_GPIO_ReadPin(ID_CHIP_GPIO_PORT, ID_CHIP_GPIO_PIN))
#define ID_CHIP_GPIO_WRITE(PIN_STATE) (HAL_GPIO_WritePin(ID_CHIP_GPIO_PORT, ID_CHIP_GPIO_PIN, PIN_STATE))

// Values taken from DS2401 datasheet
#define ID_CHIP_READ_ROM_COMMAND (0x33)
// Timing values are taken to be within sampling period
#define ID_CHIP_TRSTL (480)
#define ID_CHIP_TPDH (70)
#define ID_CHIP_TRSTH (410)
#define ID_CHIP_TRDV (9)
#define ID_CHIP_TRELEASE (55)

// a 1 is low for 0-15 us then high for 45-105 us
#define ID_CHIP_TLOW1 (6)
#define ID_CHIP_TSLOT1 (64)
// a 0 is low for 60-120 us
#define ID_CHIP_TLOW0 (60)
#define ID_CHIP_TSLOT0 (10)

static uint8_t id_chip_reset();
static void id_chip_write_byte(uint8_t byte);
static uint8_t id_chip_read_byte();
static uint8_t id_chip_crc8(uint8_t* buf, int len);

id_chip_response id_chip_get_value()
{
	// Just to be safe disable interrupts

	GPIO_InitTypeDef id_pin_init;
	id_pin_init.Pin = ID_CHIP_GPIO_PIN;
	id_pin_init.Mode = GPIO_MODE_INPUT;
	id_pin_init.Pull = GPIO_NOPULL;
	id_pin_init.Speed = GPIO_SPEED_HIGH;
	id_pin_init.Alternate = 0;
	HAL_GPIO_Init(ID_CHIP_GPIO_PORT, &id_pin_init);

	delay_us(10000);
	// Reset the chip and see if it responds
	uint8_t id_chip_exists = id_chip_reset();

	id_chip_response response = {0};
	if(id_chip_exists)
	{
		// Write the command to start the response from
		id_chip_write_byte(ID_CHIP_READ_ROM_COMMAND);
		// Read the 8 byte response
		uint8_t buf[8];
		for(int i = 0; i < 8; i++)
		{
			buf[i] = id_chip_read_byte();
		}
		// Calculate the crc of the first 7 bytes, and compare to the
		// sent crc value(the 8th byte)
		uint8_t crc = id_chip_crc8(buf,7);
		if (crc == buf[7])
		{
			response.family_id = buf[0];
			memcpy(response.serial_id, &(buf[1]),6);
		}
	}
	return response;
}

static uint8_t id_chip_reset()
{
	int retries = 20;
	GPIO_PinState read_state;
	GPIO_PinState a;

	// Wait until high
	ID_CHIP_GPIO_SET_INPUT();
	while(!(a = ID_CHIP_GPIO_READ()))
	{
		// Error return if it times out
		retries--;
		if (retries <= 0)
		{
			return 0;
		}
		delay_us(10000);
	}

	// Write Reset Pulse
	ID_CHIP_GPIO_SET_OUTPUT();
	ID_CHIP_GPIO_WRITE(0);

	delay_us(ID_CHIP_TRSTL);
	ID_CHIP_GPIO_SET_INPUT();

	// Read Response
	delay_us(ID_CHIP_TPDH);
	read_state = ID_CHIP_GPIO_READ();

	// Wait to return to high state
	delay_us(ID_CHIP_TRSTH);

	return !read_state;
}

static void id_chip_write_byte(uint8_t byte)
{
	for (int i = 0; i < 8; i++)
	{
		// Write each bit using timing from DS2401 datasheet
		uint8_t bit = byte & (0x1 << i);
		if (bit)
		{
			ID_CHIP_GPIO_SET_OUTPUT();
			ID_CHIP_GPIO_WRITE(0);
			delay_us(ID_CHIP_TLOW1);
			ID_CHIP_GPIO_SET_INPUT();
			delay_us(ID_CHIP_TSLOT1);
		}
		else
		{
			ID_CHIP_GPIO_SET_OUTPUT();
			ID_CHIP_GPIO_WRITE(0);
			delay_us(ID_CHIP_TLOW0);
			ID_CHIP_GPIO_SET_INPUT();
			delay_us(ID_CHIP_TSLOT0);
		}
	}
}

static uint8_t id_chip_read_byte()
{
	uint8_t result = 0;
	for (int i = 0; i < 8; i++)
	{
		// Write a 0 for a very short period, and read the response
		ID_CHIP_GPIO_SET_OUTPUT();
		ID_CHIP_GPIO_WRITE(0);
		delay_us(ID_CHIP_TLOW1);
		ID_CHIP_GPIO_SET_INPUT();
		delay_us(ID_CHIP_TRDV);
		result |= (ID_CHIP_GPIO_READ() << i);
		delay_us(ID_CHIP_TRELEASE);
	}
	return result;
}

// Function is taken from Arduino OneWire Library
static uint8_t id_chip_crc8(uint8_t* buf, int len)
{
	uint8_t crc = 0;
	for (int i = 0; i < len; i++)
	{
		uint8_t byte = buf[i];
		for (int j = 0; j < 8; j++)
		{
			uint8_t mix = (crc ^ byte) & 0x01;
			crc >>= 1;
			if (mix)
				crc ^= 0x8c;

			byte >>= 1;
		}
	}
	return crc;
}
