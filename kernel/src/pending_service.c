/**
 * MA-Memphis
 * @file pending_service.c
 *
 * @author Marcelo Ruaro (marcelo.ruaro@acad.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date June 2016
 *
 * @brief FIFO of incoming packets which could not be immediately handled.
 */

#include "pending_service.h"

#include "mmr.h"

list_t _psvcs;

void psvc_init()
{
	list_init(&_psvcs);
}

list_entry_t *psvc_push_back(packet_t *packet)
{
	list_entry_t *entry = list_push_back(&_psvcs, packet);
	if(entry == NULL)
		return NULL;

	MMR_PENDING_SERVICE_INTR = 1;
	return entry;
}

packet_t *psvc_front()
{
	list_entry_t *entry = list_front(&_psvcs);
	
	if(entry == NULL)
		return NULL;

	return list_get_data(entry);
}

void psvc_pop_front()
{
	list_pop_front(&_psvcs);

	if(list_empty(&_psvcs))
		MMR_PENDING_SERVICE_INTR = 0;
}

bool psvc_empty()
{
	return list_empty(&_psvcs);
}
