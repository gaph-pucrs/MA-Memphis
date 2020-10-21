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
#include "hal.h"
#include "utils.h"

pending_svc_t pending_svcs;
pending_fifo_t pending_msgs;

void pending_svc_init()
{
	pending_svcs.empty = true;
	pending_svcs.full = false;
	pending_svcs.head = 0;
	pending_svcs.tail = 0;
}

void pending_msg_init()
{
	pending_msgs.empty = true;
	pending_msgs.full = false;
	pending_msgs.head = 0;
	pending_msgs.tail = 0;
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
	HAL_PENDING_SERVICE_INTR = 1;

	return true;
}

bool pending_msg_push(int task, int size, int *msg)
{
	if(pending_msgs.full){
		puts("ERROR: Pending message FIFO FULL\n");
		while(1);
		return false;
	}

	if(size > PKG_MAX_KERNEL_MSG_LEN){
		puts("ERROR: Message too big for kernel 'pipe'\n");
		while(1);
		return false;
	}

	/* Push message to buffer */
	pending_msgs.buffer[pending_msgs.tail].task = task;
	pending_msgs.buffer[pending_msgs.tail].size = size;
	for(int i = 0; i < size; i++)
		pending_msgs.buffer[pending_msgs.tail].message[i] = msg[i];
	
	pending_msgs.tail++;

	pending_msgs.tail %= PKG_PENDING_SVC_MAX;
	pending_msgs.full = (pending_msgs.tail == pending_msgs.head);
	pending_msgs.empty = false;

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
		HAL_PENDING_SERVICE_INTR = 0;
	}

	pending_svcs.full = false;

	return packet;
}
