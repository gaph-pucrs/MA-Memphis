/**
 * MA-Memphis
 * @file message_request.c
 *
 * @author Angelo Elias Dalzotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date August 2020
 * 
 * @brief This module defines the declares request synchronization structure.
 */

#include <stddef.h>
#include <stdbool.h>

#include "message_request.h"
#include "task_control.h"
#include "services.h"
#include "packet.h"
#include "stdio.h"
#include "broadcast.h"

void mr_init(tcb_t *tcb)
{
	for(int i = 0; i < PKG_MAX_TASKS_APP; i++)
		tcb->message_request[i].requester = -1;
}

void mr_send(int producer_task, int consumer_task, int producer_addr, int consumer_addr)
{
	if(producer_task & MEMPHIS_FORCE_PORT){
		/* Message directed to peripheral, send via Hermes */
		packet_t *packet = pkt_slot_get();

		packet->header = producer_addr;
		packet->service = MESSAGE_REQUEST;
		packet->requesting_processor = consumer_addr;
		packet->producer_task = producer_task;
		packet->consumer_task = consumer_task;

		pkt_send(packet, NULL, 0);
	} else {
		br_packet_t packet;

		packet.service = MESSAGE_REQUEST;
		packet.src_id = (consumer_task & MEMPHIS_KERNEL_MSG) ? -1 : consumer_task;
		packet.cons_addr = consumer_addr;
		packet.prod_task = (producer_task & MEMPHIS_KERNEL_MSG) ? -1 : producer_task;
		// puts("Sending MESSAGE_REQUEST via BrNoC\n");
		while(!br_send(&packet, producer_addr, BR_SVC_TGT));
	}
}

bool mr_insert(tcb_t *producer_tcb, int consumer_task, int consumer_addr)
{
    for(int i = 0; i < PKG_MAX_TASKS_APP; i++){
    	if(producer_tcb->message_request[i].requester == -1){
    		producer_tcb->message_request[i].requester = consumer_task;
    		producer_tcb->message_request[i].requester_address  = consumer_addr;

    		/* Only for debug purposes */
    		MMR_ADD_REQUEST_DEBUG = (producer_tcb->id << 16) | (consumer_task & 0xFFFF);

    		return true;
		}
	}

	printf("ERROR: producer request array of task %d is full\n", producer_tcb->id);
	// while(true);
	return false;
}

message_request_t *mr_peek(tcb_t *tcb, int cons_task)
{
	for(int i = 0; i < PKG_MAX_TASKS_APP; i++){
		if(tcb->message_request[i].requester == cons_task)
			return &(tcb->message_request[i]);
	}
	return NULL;
}

void mr_pop(message_request_t *request)
{
	request->requester = -1;
}

unsigned int mr_defrag(tcb_t *tcb)
{
	/**
	 * @todo This resembles a lot the bubblesort algorithm
	 * Change this to a more efficient algorithm
	 */
	unsigned int size = 0;
	bool swapped = true;
	for(int i = 0; i < PKG_MAX_LOCAL_TASKS && swapped; i++){
		if(tcb->message_request[i].requester == -1){
			/* Slot found! */
			for(int j = i; j < PKG_MAX_LOCAL_TASKS; j++){
				swapped = false;
				if(tcb->message_request[j].requester != -1){
					tcb->message_request[i] = tcb->message_request[j];
					tcb->message_request[j].requester = -1;
					swapped = true;
					break;
				}
			}
		} else {
			size++;
		}
	}
	return size;
}
