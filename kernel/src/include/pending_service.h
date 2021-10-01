/**
 * MA-Memphis
 * @file pending_service.h
 *
 * @author Marcelo Ruaro (marcelo.ruaro@acad.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date June 2016
 *
 * @brief FIFO of incoming packets which could not be immediately handled.
 */

#pragma once

#include <stddef.h>

#include "packet.h"

/**
 * @brief FIFO data structure for pending services
 */
typedef struct _pending_svc {
	packet_t buffer[PKG_PENDING_SVC_MAX];	//!<pending services array declaration
	unsigned char head;
	unsigned char tail;
	bool empty;
	bool full;
} pending_svc_t;

/**
 * @brief Pending message information. This is a 'lightweight' pipe for kernel
 */
typedef struct _pending_msg {
	int task;
	int size;
	int message[PKG_MAX_KERNEL_MSG_LEN];
} pending_msg_t;

/**
 * @brief Initialize the pending service structures
 */
void pending_svc_init();

/**
 * @brief Initialize the pending message structures
 */
void pending_msg_init();

/**
 * @brief Adds a service to pending structure
 * 
 * @param packet Pointer to service packet
 *
 * @return True if sucess
 */
bool pending_svc_push(const volatile packet_t *packet);

/**
 * @brief Adds a service to pending structure
 * 
 * @param task ID of the target task
 * @param size Length of the message
 * @param msg Pointer to the message
 *
 * @return True if sucess
 */
bool pending_msg_push(int task, int size, int *msg);

/**
 * @brief Gets the next pending service and remove from the buffer
 * 
 * @return Pointer to the next service. NULL if no service is pending.
 */
packet_t *pending_svc_pop();

/**
 * @brief Gets a pending message pointer
 * 
 * @param id ID of the target task
 * 
 * @return Pointer to the structure found. NULL if no corresponding id
 */
pending_msg_t *pending_msg_search(int id);

/**
 * @brief Sends a kernel pending message
 * 
 * @param msg Pointer to the message structure
 * @param addr Address of the consumer task
 */ 
void pending_msg_send(pending_msg_t *msg, int addr);
