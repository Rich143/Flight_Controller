/*
 * id_chip.h
 *
 *  Created on: Feb 18, 2015
 *      Author: David Sami
 */

#ifndef ID_CHIP_H_
#define ID_CHIP_H_

typedef struct
{
	uint8_t family_id;
	uint8_t serial_id[6];
} id_chip_response;

id_chip_response id_chip_get_value();

#endif /* ID_CHIP_H_ */
