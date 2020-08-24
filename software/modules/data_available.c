/**
 * Memphis
 * 
 * @file data_available.c
 * 
 * @author Angelo Elias Dalzotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - 
 * PUCRS -
 *  
 * @brief Defines the DATA_AV fifo functions.
 */

#include <stddef.h>
#include "data_available.h"

void data_av_init(data_av_fifo_t *fifo)
{
	fifo->head = 0;
	fifo->tail = 0;
	fifo->empty = true;
	fifo->full = false;
}

bool data_av_insert(data_av_fifo_t *fifo, int producer_task, int producer_pe)
{
	if(fifo->full){
		puts("ERROR: data_av fifo full\n");
		return false;
	}

	fifo->buffer[fifo->tail].requester = producer_task;
	fifo->buffer[fifo->tail].requester_pe = producer_pe;

	fifo->tail++;
	fifo->tail %= DATA_AV_SIZE;

	if(fifo->tail == fifo->head)
		fifo->full = true;

	fifo->empty = false;

	/* Only for debug purposes */
	// MemoryWrite(ADD_DATA_AV_DEBUG, (producer_task << 16) | (consumer_task & 0xFFFF));

	return true;
}

bool data_av_pop(data_av_fifo_t *fifo)
{
	if(fifo->empty)
		return false;

	fifo->head++;
	fifo->head %= DATA_AV_SIZE;

	if(fifo->head == fifo->tail)
		fifo->empty = true;

	fifo->full = false;

	// MemoryWrite(REM_DATA_AV_DEBUG, (data_av[i].requester << 16) | (consumer_task & 0xFFFF));

	return true;
}

data_av_t *data_av_peek(data_av_fifo_t *fifo)
{
	if(fifo->empty)
		return NULL;

	return &(fifo->buffer[fifo->head]);
}