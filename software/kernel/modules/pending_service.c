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
#include "services.h"

pending_svc_t pending_svcs;
pending_msg_t pending_msgs[PKG_PENDING_SVC_MAX];

void pending_svc_init()
{
	pending_svcs.empty = true;
	pending_svcs.full = false;
	pending_svcs.head = 0;
	pending_svcs.tail = 0;
}

void pending_msg_init()
{
	for(int i = 0; i < PKG_PENDING_SVC_MAX; i++)
		pending_msgs[i].task = -1;
}

bool pending_svc_push(const volatile packet_t *packet)
{
	if(pending_svcs.full){
		puts("ERROR: Pending service FIFO FULL\n");
		while(1);
		return false;
	}

	/* Push packet to buffer */
	/* Do it manually to avoid generating a memcpy */
	pending_svcs.buffer[pending_svcs.tail].header = packet->header;
	pending_svcs.buffer[pending_svcs.tail].payload_size = packet->payload_size;
	pending_svcs.buffer[pending_svcs.tail].service = packet->service;
	pending_svcs.buffer[pending_svcs.tail].producer_task = packet->producer_task;
	pending_svcs.buffer[pending_svcs.tail].consumer_task = packet->consumer_task;
	pending_svcs.buffer[pending_svcs.tail].source_PE = packet->source_PE;
	pending_svcs.buffer[pending_svcs.tail].timestamp = packet->timestamp;
	pending_svcs.buffer[pending_svcs.tail].transaction = packet->transaction;
	pending_svcs.buffer[pending_svcs.tail].mapper_task = packet->mapper_task;
	pending_svcs.buffer[pending_svcs.tail].pkt_size = packet->pkt_size;
	pending_svcs.buffer[pending_svcs.tail].code_size = packet->code_size;
	pending_svcs.buffer[pending_svcs.tail].bss_size = packet->bss_size;
	pending_svcs.buffer[pending_svcs.tail].initial_address = packet->initial_address;

	pending_svcs.tail++;
	pending_svcs.tail %= PKG_PENDING_SVC_MAX;
	pending_svcs.full = (pending_svcs.tail == pending_svcs.head);
	pending_svcs.empty = false;

	//puts("Interruption set ON\n");
	HAL_PENDING_SERVICE_INTR = 1;

	return true;
}

bool pending_msg_push(int task, int size, int *msg)
{
	if(size > PKG_MAX_KERNEL_MSG_LEN){
		puts("ERROR: Message too big for kernel 'pipe'\n");
		while(1);
		return false;
	}

	/* Search for a free slot */
	pending_msg_t *pending = pending_msg_search(-1);

	if(!pending){
		pending = pending_msg_search_svc(MONITOR);
		if(!pending){
			puts("FATAL: No available slots in kernel 'pipe' to replace\n");
			while(1);
			return false;
		} else {
			puts("WARNING: Dropping MONITOR packet\n");
		}
	}

	/* Push message to buffer */
	pending->task = task;
	pending->size = size;
	for(int i = 0; i < size; i++)
		pending->message[i] = msg[i];

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

pending_msg_t *pending_msg_search(int id)
{
	for(int i = 0; i < PKG_PENDING_SVC_MAX; i++){
		if(pending_msgs[i].task == id)
			return &pending_msgs[i];
	}
	return NULL;
}

void pending_msg_send(pending_msg_t *msg, int addr)
{
	packet_t *packet = pkt_slot_get();

	packet->header = addr;
	packet->service = MESSAGE_DELIVERY;
	packet->producer_task = HAL_NI_CONFIG | 0x10000000;
	packet->consumer_task = msg->task;
	packet->msg_lenght = msg->size;

	/* Release pipe availability. Must check if DMNI is busy before populating again */
	msg->task = -1;

	pkt_send(packet, (unsigned*)msg->message, msg->size);
}

pending_msg_t *pending_msg_search_svc(int svc)
{
	for(int i = 0; i < PKG_PENDING_SVC_MAX; i++){
		if(pending_msgs[i].task != -1 && pending_msgs[i].message[0] == svc)
			return &pending_msgs[i];
	}
	return NULL;
}
