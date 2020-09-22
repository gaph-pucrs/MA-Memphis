/**
 * 
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

#include "pkg.h"
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
 * @brief Initialize the pending service structures
 */
void pending_svc_init();

/**
 * @brief Adds a service to pending structure
 * 
 * @param *packet Pointer to service packet
 *
 * @return True if sucess
 */
bool pending_svc_push(const packet_t *packet);

/**
 * @brief Gets the next pending service and remove from the buffer
 * 
 * @return Pointer to the next service. NULL if no service is pending.
 */
packet_t *pending_svc_pop();
