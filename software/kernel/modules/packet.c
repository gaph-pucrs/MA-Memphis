/**
 *
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
#include "hal.h"

#define PKT_SLOTS 2

enum {
	PKT_SLOT_A,
	PKT_SLOT_B
};

pkt_slot_t pkt_slots[PKT_SLOTS];		//!< Packet send control.

// unsigned int global_inst = 0;			//!<Global CPU instructions counter

const unsigned int PKT_SIZE = 13; //!<Constant Service Header size, based on the structure ServiceHeader. If you change it, please change the same define within app_injector.h

void pkt_init()
{
	for(int i = 0; i < PKT_SLOTS; i++)
		pkt_slots[i].free = true;
}

packet_t pkt_read(){
	volatile packet_t packet;
	
	HAL_DMNI_SIZE = PKT_SIZE;
	HAL_DMNI_OP = HAL_DMNI_WRITE;
	HAL_DMNI_ADDRESS = (unsigned int)&packet;
	HAL_DMNI_START = 1;
	
	/* Wait for data transfer */
	while(HAL_DMNI_RECEIVE_ACTIVE);

	return packet;
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

	packet->payload_size = (PKT_SIZE - 2) + size;
	packet->transaction = 0;
	packet->source_PE = HAL_NI_CONFIG;

	/* Wait for DMNI be release */
	while(HAL_DMNI_SEND_ACTIVE);

	packet->timestamp = HAL_TICK_COUNTER;

	HAL_DMNI_SIZE = PKT_SIZE;
	HAL_DMNI_ADDRESS = (unsigned int)packet;

	if(size > 0){
		HAL_DMNI_SIZE_2 = size;
		HAL_DMNI_ADDRESS_2 = (unsigned int)buffer;
	}

	HAL_DMNI_OP = HAL_DMNI_READ;
	HAL_DMNI_START = 1;
}
