/**
 * MA-Memphis
 * @file broadcast.c
 *
 * @author Angelo Elias Dalzotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date September 2021
 * 
 * @brief API for Broadcast Network communication
 */

#include "broadcast.h"

#include "mmr.h"
#include "memphis.h"
#include "services.h"
#include "stdio.h"

bool br_send(br_packet_t *packet, int16_t tgt_addr, uint8_t service)
{
	if(MMR_BR_LOCAL_BUSY)
		return false;

	MMR_BR_SERVICE = service & 0x7;
	MMR_BR_KSVC = packet->service;
	MMR_BR_PRODUCER = packet->src_id;
	MMR_BR_TARGET = tgt_addr;
	MMR_BR_PAYLOAD = packet->payload;
	
	MMR_BR_START = 1;
	return true;
}

void br_read(br_packet_t *packet)
{
	packet->service = MMR_BR_READ_KSVC;

	uint32_t producer = MMR_BR_READ_PRODUCER;
	packet->src_addr = producer >> 16;
	packet->src_id = producer & 0xFFFF;
	
	uint32_t message = MMR_BR_READ_PAYLOAD;
	packet->prod_addr = message >> 16;
	packet->cons_task = message & 0xFFFF;
}

void br_fake_packet(br_packet_t *br_packet, packet_t *packet)
{
	packet->service = br_packet->service;

	switch(packet->service){
		case DATA_AV:
			packet->producer_task = br_convert_id(br_packet->src_id, br_packet->prod_addr);
			packet->consumer_task = br_convert_id(br_packet->cons_task, MMR_NI_CONFIG);
			packet->requesting_processor = br_packet->prod_addr;
			break;
		case MESSAGE_REQUEST:
			packet->producer_task = br_convert_id(br_packet->prod_task, MMR_NI_CONFIG);
			packet->consumer_task = br_convert_id(br_packet->src_id, br_packet->cons_addr);
			packet->requesting_processor = br_packet->cons_addr;
			break;
		case TASK_MIGRATION:
			packet->task_ID = br_packet->task_id;
			packet->allocated_processor = br_packet->target_pe;
			break;
		default:
			printf("ERROR: Service %x not implemented in br->hermes\n", packet->service);
			break;
	}
}

int br_convert_id(int16_t id, int16_t addr)
{
	return (id == -1) ? (MEMPHIS_KERNEL_MSG | (int)addr) : id;
}
