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
#include "services.h"
#include "dmni.h"

enum {
	PKT_SLOT_A,
	PKT_SLOT_B,
	PKT_SLOT_MAX
};

pkt_slot_t pkt_slots[PKT_SLOT_MAX];	//!< Packet send control.

void pkt_init()
{
	for(int i = 0; i < PKT_SLOT_MAX; i++)
		pkt_slots[i].free = true;
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

void pkt_set_message_delivery(packet_t *packet, int consumer_addr, int producer_task, int consumer_task, size_t size)
{
	packet->header = consumer_addr;
	packet->service = MESSAGE_DELIVERY;
	packet->producer_task = producer_task;
	packet->consumer_task = consumer_task;
	packet->msg_length = size;
	packet->insert_request = MMR_NI_CONFIG;
}

void pkt_set_migration_pipe(packet_t *packet, int addr, int producer_task, int consumer_task, int size)
{
	packet->header = addr;
	packet->task_ID = producer_task;
	packet->service = MIGRATION_PIPE;
	packet->consumer_task = consumer_task;
	packet->msg_length = size;
}

void pkt_set_dmni_info(packet_t *packet, size_t payload_size)
{
	packet->payload_size = (PKT_SIZE - 2) + payload_size;
	packet->transaction = 0;
	packet->source_PE = MMR_NI_CONFIG;
	packet->timestamp = MMR_TICK_COUNTER;
}
