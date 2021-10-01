/**
 * MA-Memphis
 * @file dmni.c
 * 
 * @author Angelo Elias Dalzotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date September 2020
 * 
 * @brief Defines the DMNI functions for payload handling.
 */

#include "dmni.h"
#include "mmr.h"

void dmni_read(unsigned int *payload_address, int payload_size)
{
	MMR_DMNI_SIZE = (unsigned int)payload_size;
	MMR_DMNI_OP = DMNI_WRITE;
	MMR_DMNI_ADDRESS = (unsigned int)payload_address;
	MMR_DMNI_START = 1;
	while(MMR_DMNI_RECEIVE_ACTIVE);
}
