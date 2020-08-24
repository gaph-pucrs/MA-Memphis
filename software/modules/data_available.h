/**
 * Memphis
 * 
 * @file data_available.h
 * 
 * @author Angelo Elias Dalzotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 *  
 * @brief Declares the DATA_AV fifo structure and functions.
 */

#ifndef _DATA_AVAILABLE_
#define _DATA_AVAILABLE_

#include <stdbool.h>
#include "communication.h"

#define DATA_AV_SIZE	PIPE_SIZE*(MAX_TASKS_APP-1)

typedef struct _data_av {
	int requester;
	int requester_pe;
} data_av_t;

typedef struct _data_av_fifo {
	data_av_t buffer[DATA_AV_SIZE];
	unsigned int head;
	unsigned int tail;
	bool empty;
	bool full;
} data_av_fifo_t;

/**
 * @brief Initializes the DATA_AV structure.
 * 
 * @param *fifo Pointer to the structure to be initialized
 */
void data_av_init(data_av_fifo_t *fifo);

/**
 * @brief Inserts a data_av into the page DATA_AV fifo
 * 
 * @param *fifo Pointer to the structure to be inserted
 * @param producer_task ID of the producer task of the message
 * @param producer_pe Processor of the producer task
 * 
 * @return false if the data_av array is full, true if the message was successfully inserted
 */
bool data_av_insert(data_av_fifo_t *fifo, int producer_task, int producer_pe);

/**
 * @brief Removes the oldest element from the FIFO
 * 
 * @param *fifo Pointer to the structure to be removed
 * 
 * @return True if element is removed, false if not
 */
bool data_av_pop(data_av_fifo_t *fifo);

/**
 * @brief Peek the oldest element from the fifo
 * 
 * @param *fifo Pointer to the structure to be peeked
 * 
 * @return Pointer to the oldest element or null if empty
 */
data_av_t *data_av_peek(data_av_fifo_t *fifo);

#endif