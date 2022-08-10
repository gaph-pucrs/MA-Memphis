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

#include <stdlib.h>

#include "dmni.h"
#include "mmr.h"

void dmni_read(void *payload_address, size_t payload_size)
{
	MMR_DMNI_SIZE = (unsigned int)payload_size;
	MMR_DMNI_OP = DMNI_WRITE;
	MMR_DMNI_ADDRESS = (unsigned int)payload_address;
	MMR_DMNI_START = 1;
	while(MMR_DMNI_RECEIVE_ACTIVE);
}

void dmni_send(packet_t *packet, void *payload, size_t size)
{
	static void *outbound = NULL;

	/* Wait for DMNI be released */
	while(MMR_DMNI_SEND_ACTIVE);

	if(outbound != NULL)
		free(outbound);

	outbound = payload;

	/* Program DMNI */
	MMR_DMNI_SIZE = PKT_SIZE;
	MMR_DMNI_ADDRESS = (unsigned)packet;

	MMR_DMNI_SIZE_2 = size;
	MMR_DMNI_ADDRESS_2 = (unsigned)outbound;

	MMR_DMNI_OP = DMNI_READ;

	pkt_set_dmni_info(packet, size);

	MMR_DMNI_START = 1;
}
