/**
 * MA-Memphis
 * @file broadcast.h
 *
 * @author Angelo Elias Dalzotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date September 2021
 * 
 * @brief API for Broadcast Network communication
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Sends a message via BrNoC
 * 
 * @param payload Message to send
 * @param producer ID of the producer task
 * @param target PE address to send the message to
 * @param ksvc Kernel service used in ALL and TARGET messages (see services.h)
 * @param service Broadcast service (ALL/TARGET/MONITORS)
 * 
 * @return True if success. False if BrNoC is busy.
 */
bool br_send(uint32_t payload, uint16_t producer, uint16_t target, uint8_t ksvc, uint8_t service);
