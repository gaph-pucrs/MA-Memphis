/**
 * 
 * @file pipe.c
 *
 * @author Angelo Elias Dalzotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date August 2020
 * 
 * @brief This module defines the pipe message structure.
 */

#include <stddef.h>

#include "pipe.h"
#include "task_control.h"
#include "services.h"
#include "packet.h"
#include "stdio.h"

void pipe_init(tcb_t *tcb)
{
	tcb->pipe.consumer_task = -1;
}

pipe_t *pipe_pop(tcb_t *producer_tcb, int consumer_task)
{
	if(producer_tcb->pipe.consumer_task == consumer_task){
		/* Only for debug purposes */
		MMR_REM_PIPE_DEBUG = (producer_tcb->id << 16) | (consumer_task & 0xFFFF);

		producer_tcb->pipe.consumer_task = -1;

		return &(producer_tcb->pipe);
	} else {
		return NULL;
	}
}

void pipe_send(int producer_task, int consumer_task, int consumer_addr, pipe_t *message)
{
	packet_t *packet = pkt_slot_get();

	packet->header = consumer_addr;
	packet->service = MESSAGE_DELIVERY;
	packet->producer_task = producer_task;
	packet->consumer_task = consumer_task;
	packet->msg_lenght = message->message.length;

	/* Release pipe availability. Must check if DMNI is busy before populating again */
	message->consumer_task = -1;

	if(message->message.length > PKG_MAX_MSG_SIZE){
		printf("ERROR: message length of %d higher than MSG_SIZE\n", packet->msg_lenght);
		while(true);
	}
	// putsv("Sending with message len ", packet->msg_lenght);

	pkt_send(packet, (unsigned int*)message->message.payload, message->message.length);
}

bool pipe_is_full(tcb_t *tcb)
{
	return tcb->pipe.consumer_task != -1;
}

void pipe_transfer(message_t *src, message_t *dst)
{
	dst->length = src->length;
	/** @todo Memcpy */
	for(volatile int i = 0; i < dst->length; i++)
		dst->payload[i] = src->payload[i];
}

bool pipe_push(tcb_t *tcb, message_t *message, int cons_task)
{
	if(tcb->pipe.consumer_task != -1)
		return false;

	tcb->pipe.consumer_task = cons_task;
	tcb->pipe.message.length = message->length;
	// putsv("Pipe message length = ", tcb->pipe.message.length);
	/** @todo memcpy */
	for(volatile int i = 0; i < message->length; i++)
		tcb->pipe.message.payload[i] = message->payload[i];

	return true;
}

message_t *pipe_get_message(pipe_t *pipe)
{
	return &(pipe->message);
}

int pipe_get_cons_task(tcb_t *tcb)
{
	return tcb->pipe.consumer_task;
}

void pipe_set_cons_task(tcb_t *tcb, int cons_task)
{
	tcb->pipe.consumer_task = cons_task;
}

unsigned int pipe_get_message_len(tcb_t *tcb)
{
	return tcb->pipe.message.length;
}

void pipe_set_message_len(tcb_t *tcb, unsigned int len)
{
	tcb->pipe.message.length = len;
}
