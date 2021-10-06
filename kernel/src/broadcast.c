/**
 * MA-Memphis
 * @file broadcast.c
 *
 * @author Angelo Elias Dalzotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date September 2021
 * 
 * @brief API for Broadcast Network communication
 */

#include "broadcast.h"

#include "mmr.h"

bool br_send(uint32_t payload, uint16_t producer, uint16_t target, uint8_t ksvc, uint8_t service)
{
	if(MMR_BR_LOCAL_BUSY)
		return false;

	MMR_BR_PAYLOAD = payload;
	MMR_BR_TARGET = target;
	MMR_BR_SERVICE = service & 0x7;
	MMR_BR_PRODUCER = producer;
	MMR_BR_KSVC = ksvc;
	MMR_BR_START = 1;
	return true;
}
