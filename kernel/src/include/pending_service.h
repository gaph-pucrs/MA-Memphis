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

#include <mutils/list.h>

#include "packet.h"

/**
 * @brief Initialize the pending service structures
 */
void psvc_init();

/**
 * @brief Adds a service to pending structure
 * 
 * @param packet Pointer to service packet
 *
 * @return list_entry_t* Pointer to entry
 */
list_entry_t *psvc_push_back(packet_t *packet);

/**
 * @brief Gets the first element from the fifo
 * 
 * @return packet_t* Pointer to a packet
 */
packet_t *psvc_front();

/**
 * @brief Removes the first element from the FIFO
 */
void psvc_pop_front();

/**
 * @brief Checks if there are no more pending services
 * 
 * @return true No more pending services
 * @return false Has pending services
 */
bool psvc_empty();
