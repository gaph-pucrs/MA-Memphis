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

#include <stdint.h>
#include <stdbool.h>

#include "packet.h"

typedef struct _bcast {
	uint8_t service;

	int16_t src_addr;
	int16_t src_id;

	uint32_t payload;
} bcast_t;

/**
 * @brief Sends a message via BrNoC
 * 
 * @param packet Pointer to packet to send -- will be copied, no need to be static
 * @param tgt_addr PE address to send the message to
 * @param service Broadcast service (ALL/TARGET/MONITORS)
 * 
 * @return True if success. False if BrNoC is busy.
 */
bool bcast_send(bcast_t *packet, int16_t tgt_addr, uint8_t service);

/**
 * @brief Reads a packet via BrNoC
 * 
 * @param packet Pointer to packet to save
 */
void bcast_read(bcast_t *packet);

/**
 * @brief Fakes a broadcast packet into a Hermes packet
 * 
 * @param bcast_packet Pointer to the BrNoC packet (source)
 * @param packet Pointer to the Hermes packet (target)
 */
void bcast_fake_packet(bcast_t *bcast_packet, packet_t *packet);

/**
 * @brief Convert an ID into a 32 bit field with KERNEL flag
 * 
 * @param id Received ID from BrNoC
 * @param addr Received address from BrNoC
 * 
 * @return 32-bit ID
 */
int bcast_convert_id(int16_t id, int16_t addr);
