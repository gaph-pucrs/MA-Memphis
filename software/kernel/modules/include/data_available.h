/**
 * 
 * @file data_available.h
 * 
 * @author Angelo Elias Dalzotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date August 2020
 * 
 * @brief Declares the DATA_AV fifo structure and functions.
 */

#pragma once

#include <stdbool.h>

#include "task_control.h"
#include "pkg.h"

typedef struct _data_av {
	int requester;
	int requester_addr;
} data_av_t;

typedef struct _data_av_fifo {
	data_av_t buffer[PKG_MAX_TASKS_APP];
	unsigned int head;
	unsigned int tail;
	bool empty;
	bool full;
} data_av_fifo_t;

/**
 * @brief Initializes the DATA_AV structure.
 * 
 * @param tcb Pointer to the tcb structure to initialize
 */
void data_av_init(tcb_t *tcb);

/**
 * @brief Inserts a data_av into the page DATA_AV fifo
 * 
 * @param tcb Pointer to the TCB that will be inserted
 * @param producer_task ID of the producer task of the message
 * @param producer_addr Processor of the producer task
 * 
 * @return False if the data_av array is full, true if the message was successfully inserted
 */
bool data_av_insert(tcb_t *tcb, int producer_task, int producer_addr);

/** 
 * @brief Assembles and sends a DATA_AV packet to producer task
 * 
 * @param consumer_task Consumer task ID (Receive())
 * @param producer_task Producer task ID (Send())
 * @param consumer_addr Processor address of the consumer task
 * @param producer_addr Processor address of the producer task
 */
void data_av_send(int consumer_task, int producer_task, int consumer_addr, int producer_addr);

/**
 * @brief Removes the oldest element from the FIFO
 * 
 * @param tcb Pointer to the TCB
 * 
 * @return Pointer to the oldest element. NULL if fifo is empty
 */
data_av_t *data_av_pop(tcb_t *tcb);

/**
 * @brief Peek the oldest element from the fifo
 * 
 * @param tcb Pointer to the TCB
 * 
 * @return Pointer to the oldest element or null if empty
 */
data_av_t *data_av_peek(tcb_t *tcb);
