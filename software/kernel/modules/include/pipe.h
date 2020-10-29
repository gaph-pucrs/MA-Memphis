/**
 * 
 * @file pipe.h
 *
 * @author Angelo Elias Dalzotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date August 2020
 * 
 * @brief This module declares the pipe message structure.
 */

#pragma once

#include <stdbool.h>

/** @todo move to a proper API header */
#define MSG_SIZE 128

/* Forward declaration */
typedef struct _tcb tcb_t;

typedef struct _message {
	// unsigned short length;
	int length;
	// unsigned int msg[MSG_SIZE];
	int msg[MSG_SIZE];
} message_t;

/**
 * @brief This structure store a task message (Message) in a kernel memory area called PIPE
 */
typedef struct _pipe {
	int consumer_task;	//!< Stores consumer task id (task that performs the Receive() API )
	message_t message;	//!< Stores the message itself - Message is a structure defined into api.h
} pipe_t;

/**
 * @brief Initializes the task's pipe.
 * 
 * @param tcb Pointer to the TCB to clear the pipe.
 */
void pipe_init(tcb_t *pipe);

/**
 * @brief Gets the element in the pipe
 * 
 * @param producer_tcb Pointer to the producer task TCB
 * @param consumer_task ID of the consumer task
 * 
 * @return Pointer to the pipe structure. If empty, returns NULL.
 */
pipe_t *pipe_pop(tcb_t *producer_tcb, int consumer_task);

/**
 * @brief Assembles and sends a MESSAGE_DELIVERY packet to a consumer task.
 * 
 * @param producer_task ID of the producer task.
 * @param consumer_task ID of the consumer task.
 * @param consumer_addr Address of the consumer task.
 * @param message		Pointer to the pipe that will be sent.
 */
void pipe_send(int producer_task, int consumer_task, int consumer_addr, pipe_t *message);

/**
 * @brief Checks if there is still a message to send
 * 
 * @param tcb Pointer of the TCB
 * 
 * @return True if pipe is full
 */
bool pipe_is_full(tcb_t *tcb);

/**
 * @brief Transfers a message from producer to consumer
 * 
 * @param src Pointer to message source
 * @param dst Pointer to message destination
 */
void pipe_transfer(message_t *src, message_t *dst);

/**
 * @brief Inserts a message in pipe
 * 
 * @param tcb Pointer to the TCB to insert
 * @param message Pointer to the message to insert
 * @param cons_task ID of the consumer task
 * 
 * @return True if inserted. False if pipe has value.
 */
bool pipe_push(tcb_t *tcb, message_t *message, int cons_task);

/**
 * @brief Gets the message from a pipe
 * 
 * @param pipe Pointer to pipe
 * 
 * @return Pointer to message
 */
message_t *pipe_get_message(pipe_t *pipe);

/**
 * @brief Gets the consumer task of the message in pipe
 * 
 * @param tcb Pointer to the TCB
 * 
 * @return ID of the consumer task
 */
int pipe_get_cons_task(tcb_t *tcb);

/**
 * @brief Sets the consumer task of the message in pipe
 * 
 * @param tcb Pointer to the TCB
 * @param cons_task ID of the consumer task
 */
void pipe_set_cons_task(tcb_t *tcb, int cons_task);

/**
 * @brief Gets the message length in pipe
 * 
 * @param tcb Pointer to the TCB
 * 
 * @return Value of the length of the message
 */
unsigned int pipe_get_message_len(tcb_t *tcb);

/**
 * @brief Sets the message length in pipe
 * 
 * @param tcb Pointer to the TCB
 * @param len Length of the message
 */
void pipe_set_message_len(tcb_t *tcb, unsigned int len);
