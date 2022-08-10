/**
 * MA-Memphis
 * @file pending_msg.h
 *
 * @author Angelo Elias Dal Zotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date August 2022
 *
 * @brief Outgoing kernel messages encapsulated in the Memphis messaging API.
 */

#pragma once

#include "opipe.h"

/**
 * @brief Initializes the pending message FIFO
 */
void pend_msg_init();

/**
 * @brief Pushes a pending message to the FIFO
 * 
 * @param buf Pointer to the message
 * @param size Size of the message
 * @param cons_task Consumer task of the message
 * 
 * @return int Number of bytes in the message
 */
int pend_msg_push(void *buf, size_t size, int cons_task);

/**
 * @brief Finds a message in the pending FIFO
 * 
 * @param cons_task Consumer task ID of the message
 * 
 * @return opipe_t* Pointer to an output pipe
 */
opipe_t *pend_msg_find(int cons_task);

/**
 * @brief Removes an output pipe from the pending messages
 * 
 * @param pending Pointer to the output pipe
 */
void pend_msg_remove(opipe_t *pending);
