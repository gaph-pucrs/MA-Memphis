/**
 * MA-Memphis
 * @file opipe.h
 *
 * @author Angelo Elias Dalzotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date August 2022
 * 
 * @brief This module declares the output pipe message structure.
 */

#pragma once

#include <stddef.h>

/**
 * @brief This structure stores a task message in kernel space (task -- kernel -> NoC)
 */
typedef struct _opipe {
	int consumer_task;
	void *buf;
	size_t size;
} opipe_t;

/**
 * @brief Pushes a message to the output pipe, allocating memory
 * 
 * @param opipe Pointer to the output pipe structure
 * @param msg Pointer to source message to copy to the pipe
 * @param size Size of the message to copy
 * @param prod_task Source consumer task ID
 * @param cons_task Target consumer task ID
 * 
 * @return int Number of bytes copied
 */
int opipe_push(opipe_t *opipe, void *msg, size_t size, int prod_task, int cons_task);

/**
 * @brief Gets the buffer pointer to opipe
 * 
 * @param opipe Pointer to opipe structure
 * @param size Pointer to size variable to hold opipe size
 * @return void* Pointer to buf
 */
void *opipe_get_buf(opipe_t *opipe, size_t *size);

/**
 * @brief Pops the pipe
 * 
 * @details This frees the message in buffer
 * 
 * @param opipe Pointer to the pipe
 */
void opipe_pop(opipe_t *opipe);

/**
 * @brief Sends the message stored in pipe through DMNI
 * 
 * @param opipe Pointer to the pipe
 * @param producer_task ID of the producer task
 * @param consumer_addr Address of the consumer task
 */
void opipe_send(opipe_t *opipe, int producer_task, int consumer_addr);

/**
 * @brief Gets the consumer task of the pipe
 * 
 * @param opipe Pointer to the pipe
 * 
 * @return int The target task ID
 */
int opipe_get_cons_task(opipe_t *opipe);

/**
 * @brief Transfers a message from the output pipe to a buffer
 * 
 * @param opipe Pointer to the pipe
 * @param dst Pointer to the destination buffer
 * @param size Size of the destination buffer
 * 
 * @return size_t Number of bytes transferred
 */
size_t opipe_transfer(opipe_t *opipe, void *dst, size_t size);

/**
 * @brief Receives a migrated pipe
 * 
 * @param opipe Pointer to the pipe
 * @param size Size of the migrated message
 * @param cons_task ID of the consumer task
 * 
 * @return int Number of bytes received
 */
int opipe_receive(opipe_t *opipe, size_t size, int cons_task);
