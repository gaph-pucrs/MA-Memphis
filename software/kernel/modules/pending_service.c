/**
 * 
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
#include "pkg.h"

pending_svc_t pending_svcs;

void pending_svc_init()
{
	pending_svcs.empty = true;
	pending_svcs.full = false;
	pending_svcs.head = 0;
	pending_svcs.tail = 0;
}

bool pending_svc_push(const packet_t *packet){

	if(pending_svcs.full){
		puts("ERROR: Pending service FIFO FULL\n");
		while(1);
		return false;
	}

	/* Push packet to buffer */
	pending_svcs.buffer[pending_svcs.tail++] = *packet;
	pending_svcs.tail %= PKG_PENDING_SVC_MAX;
	pending_svcs.full = (pending_svcs.tail == pending_svcs.head);
	pending_svcs.empty = false;

	//puts("Interruption set ON\n");
	*HAL_PENDING_SERVICE_INTR = 1;

	return true;
}

packet_t *pending_svc_pop()
{
	if(pending_svcs.empty)
		return NULL;

	packet_t *packet = &pending_svcs.buffer[pending_svcs.head++];

	pending_svcs.head %= PKG_PENDING_SVC_MAX;
	if(pending_svcs.head == pending_svcs.tail){
		pending_svcs.empty = true;

		/* No more pending packets. Unset interrupt flag */
		*HAL_PENDING_SERVICE_INTR = 0;
	}

	pending_svcs.full = false;

	return packet;
}
