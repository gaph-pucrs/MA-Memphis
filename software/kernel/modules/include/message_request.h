/**
 * 
 * @file message_request.h
 *
 * @author Angelo Elias Dalzotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date August 2020
 * 
 * @brief This module declares the message request synchronization structure.
 */

#pragma once

/* Forward Declaration */
typedef struct _tcb tcb_t;

/**
 * @brief This structure stores the message requests used to implement the blocking Receive MPI
 */
typedef struct _message_request {
    int requester;			//!< Store the requested task id ( task that performs the Receive() API )
    int requester_address;	//!< Stores the requester processor address
} message_request_t;

/**
 * @brief Initialize the message request structure of the task
 * 
 * @param tcb Pointer to the TCB to clear the message request
 */
void mr_init(tcb_t *tcb);

/**
 * @brief Sends a MESSAGE_REQUEST through the NoC
 * 
 * @param producer_task ID of the producer task (receiver)
 * @param consumer_task ID of the consumer task (sender)
 * @param producer_addr Address of the producer task (receiver)
 * @param consumer_addr Address of the consumer task (sender)
 */
void mr_send(int producer_task, int consumer_task, int producer_addr, int consumer_addr);

/**
 * @brief Inserts a message request to a TCB
 * 
 * @param producer_tcb Pointer to the producer TCB (where the message will be inserted)
 * @param consumer_task ID of the requesting task
 * @param consumer_addr Address of the requesting task
 * 
 * @return True if value inserted. False if array is full.
 */
bool mr_insert(tcb_t *producer_tcb, int consumer_task, int consumer_addr);

/**
 * @brief Gets a message request
 * 
 * @param tcb Pointer to the producer tcb
 * @param cons_task ID of the consumer task
 *
 * @return Pointer to the message. NULL if not found.
 */
message_request_t *mr_peek(tcb_t *tcb, int cons_task);

/**
 * @brief Invalidates the message request
 * 
 * @param request Pointer to the request structure
 */
void mr_pop(message_request_t *request);

/**
 * @brief Defragments the message request structure
 * 
 * @param tcb Pointer to the TCB
 * 
 * @return Number of pending requests
 */
hal_word_t mr_defrag(tcb_t *tcb);
