/**
 * MA-Memphis
 * @file packet.c
 *
 * @author Marcelo Ruaro (marcelo.ruaro@acad.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date June 2016
 *
 * @brief This module defines the Packet structure. 
 * 
 * @details This structure is used by all software components to send and 
 * receive packets. It defines the service header of each packets.
 */

#include "packet.h"
#include "mmr.h"
#include "dmni.h"
#include <stdint.h>
#include <stdio.h>

#define PKT_SLOTS 2

enum {
	PKT_SLOT_A,
	PKT_SLOT_B
};

pkt_slot_t pkt_slots[PKT_SLOTS];		//!< Packet send control.

void pkt_init()
{
	for(int i = 0; i < PKT_SLOTS; i++)
		pkt_slots[i].free = true;
}

void pkt_read(volatile packet_t *packet)
{	
	MMR_DMNI_SIZE = PKT_SIZE;
	MMR_DMNI_OP = DMNI_WRITE;
	MMR_DMNI_ADDRESS = (unsigned int)packet;
	MMR_DMNI_START = 1;
	
	/* Wait for data transfer */
	while(MMR_DMNI_RECEIVE_ACTIVE);
}

packet_t *pkt_slot_get()
{
	if(pkt_slots[PKT_SLOT_A].free){
		pkt_slots[PKT_SLOT_A].free = false;
		pkt_slots[PKT_SLOT_B].free = true;
		return &pkt_slots[PKT_SLOT_A].service_header;
	} else {
		pkt_slots[PKT_SLOT_A].free = true;
		pkt_slots[PKT_SLOT_B].free = false;
		return &pkt_slots[PKT_SLOT_B].service_header;
	}
}

void pkt_send(packet_t *packet, unsigned int *buffer, unsigned int size){

    //for (genvar i = 0; i < 8; i++)
	//    assign addressChecksum[i] = A[2*i] ^ A[2*i + 1] ^ checksumIV[2*i] ^ checksumIV[2*i + 1];
    //
    //for (genvar i = 0; i < 8; i++)
	//    assign sizeChecksum[i] = A[3*i] ^ A[3*i + 1] ^ A[3*i + 2] ^ B[i+16];

    const uint16_t ChecksumIV = (PKG_N_PE_X << 8) | PKG_N_PE_Y;

    uint8_t addr_checksum = 0;
    uint8_t size_checksum = 0;

    //printf("checksum_partial: %x\n", checksum_partial);
    //printf("addr flit: %x\n", packet->header);

    for (uint8_t i = 0; i < 8; i++) {

        uint8_t addr_checksum_bit;

        addr_checksum_bit = 0;

        addr_checksum_bit ^= (packet->header & (1 << 2*i)) >> 2*i;
        addr_checksum_bit ^= (packet->header & (1 << (2*i + 1))) >> (2*i + 1);
        addr_checksum_bit ^= (ChecksumIV & (1 << 2*i)) >> 2*i;
        addr_checksum_bit ^= (ChecksumIV & (1 << (2*i + 1))) >> (2*i + 1);

        //printf("addr_checksum_bit[%x]: %x\n", i, addr_checksum_bit);

        addr_checksum ^= addr_checksum_bit << i;

    }

    //printf("addr_checksum: %x\n", addr_checksum);

    // ADDR checksum at bits [23:16]
	packet->header = (packet->header & 0xFF00FFFF) ^ (addr_checksum << 16);

	packet->payload_size = (PKT_SIZE-2) + size;

    //printf("size flit: %x\n", packet->payload_size);

    for (uint32_t i = 0; i < 8; i++) {

        uint8_t size_checksum_bit;

        size_checksum_bit = 0;

        size_checksum_bit ^= (packet->payload_size & (1 << 3*i)) >> 3*i;
        size_checksum_bit ^= (packet->payload_size & (1 << (3*i + 1))) >> (3*i + 1);
        size_checksum_bit ^= (packet->payload_size & (1 << (3*i + 2))) >> (3*i + 2);
        size_checksum_bit ^= (addr_checksum & (1 << i)) >> i;

        //printf("size_checksum_bit[%x]: %x\n", i, size_checksum_bit);

        size_checksum ^= size_checksum_bit << i;

    }

    //printf("size_checksum: %x\n", size_checksum);

    // SIZE checksum at bits [31:24]
	packet->payload_size = (packet->payload_size & 0x00FFFFFF) ^ (size_checksum << 24);

	packet->transaction = 0;
	packet->source_PE = MMR_NI_CONFIG;

	/* Wait for DMNI be release */
	while(MMR_DMNI_SEND_ACTIVE);

	packet->timestamp = MMR_TICK_COUNTER;

	MMR_DMNI_SIZE = PKT_SIZE;
	MMR_DMNI_ADDRESS = (unsigned int) packet;

	if(size > 0){
		MMR_DMNI_SIZE_2 = size;
		MMR_DMNI_ADDRESS_2 = (unsigned int) buffer;
	}

	MMR_DMNI_OP = DMNI_READ;
	MMR_DMNI_START = 1;
}
