/**
 * 
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

#include "message_request.h"
#include "pkg.h"
#include "services.h"
#include "packet.h"

void mr_init(tcb_t *tcb)
{
	for(int i = 0; i < PKG_MAX_TASKS_APP; i++)
		tcb->message_request[i].requester = -1;
}

void mr_send(int producer_task, int consumer_task, int producer_addr, int consumer_addr)
{
	packet_t *packet = pkt_slot_get();

	packet->header = producer_addr;
	packet->service = MESSAGE_REQUEST;
	packet->requesting_processor = consumer_addr;
	packet->producer_task = producer_task;
	packet->consumer_task = consumer_task;

	pkt_send(packet, NULL, 0);
}

bool mr_insert(tcb_t *producer_tcb, int consumer_task, int consumer_addr)
{
    for(int i = 0; i < PKG_MAX_TASKS_APP; i++){
    	if(producer_tcb->message_request[i].requester == -1){
    		producer_tcb->message_request[i].requester = consumer_task;
    		producer_tcb->message_request[i].requester_address  = consumer_addr;

    		/* Only for debug purposes */
    		*HAL_ADD_REQUEST_DEBUG = (producer_tcb->id << 16) | (consumer_task & 0xFFFF);

    		return true;
		}
	}

    puts("ERROR: producer request array full\n");
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

hal_word_t mr_defrag(tcb_t *tcb)
{
	hal_word_t size = PKG_MAX_LOCAL_TASKS;
	hal_word_t last_found = PKG_MAX_LOCAL_TASKS;
	for(int i = 0; i < last_found; i++){
		if(tcb->message_request[i].requester == -1){
			/* Slot found */
			for(int j = last_found - 1; j > i; j++){
				size = j;
				if(tcb->message_request[j].requester != -1){
					last_found = j;
					tcb->message_request[i] = tcb->message_request[j];
				}
			}
		}
	}
	return size;
}
