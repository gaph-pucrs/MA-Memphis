/**
 * MA-Memphis
 * @file opipe.c
 *
 * @author Angelo Elias Dalzotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date August 2022
 * 
 * @brief This module defines the output pipe message structure.
 */

#include "opipe.h"

#include <stdlib.h>
#include <string.h>

#include "mmr.h"
#include "dmni.h"

int opipe_push(opipe_t *opipe, void *msg, size_t size, int prod_task, int cons_task)
{
	size_t align_size = (size + 3) & ~3;

	opipe->buf = malloc(align_size);

	if(opipe->buf == NULL)
		return -1;

	opipe->consumer_task = cons_task;
	opipe->size = size;
	memcpy(opipe->buf, msg, size);

	size_t padding_size = align_size - size;

	for(int i = align_size - padding_size; i < align_size; i++)
		((char*)opipe->buf)[i] = padding_size;

	MMR_ADD_PIPE_DEBUG = (prod_task << 16) | (cons_task & 0xFFFF);

	return size;
}

void *opipe_get_buf(opipe_t *opipe, size_t *size)
{
	if(size != NULL)
		*size = opipe->size;

	return opipe->buf;
}

void opipe_pop(opipe_t *opipe)
{
    free(opipe->buf);
    opipe->buf = NULL;
}

void opipe_send(opipe_t *opipe, int producer_task, int consumer_addr)
{
	packet_t *packet = pkt_slot_get();

	pkt_set_message_delivery(
		packet, 
		consumer_addr, 
		producer_task, 
		opipe->consumer_task, 
		opipe->size
	);

	size_t align_size = (opipe->size + 3) & ~3;

	dmni_send(packet, opipe->buf, align_size >> 2, true);
}

int opipe_get_cons_task(opipe_t *opipe)
{
	return opipe->consumer_task;
}

size_t opipe_transfer(opipe_t *opipe, void *dst, size_t size)
{
	if(opipe->buf == NULL)
		return -1;

	if(size < opipe->size)
		return 0;

	memcpy(dst, opipe->buf, opipe->size);

	return opipe->size;
}

int opipe_receive(opipe_t *opipe, size_t size, int cons_task)
{
	size_t align_size = (size + 3) & ~3;

	opipe->buf = malloc(align_size);

	if(opipe->buf == NULL)
		return -1;

	opipe->consumer_task = cons_task;
	opipe->size = size;

	dmni_read(opipe->buf, align_size >> 2);

	return size;
}
