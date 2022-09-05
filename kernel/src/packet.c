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

#include <stdbool.h>

#include <memphis/services.h>

#include "mmr.h"

/**
 * @brief Enumerates the slots
 */
enum {
	PKT_SLOT_A,
	PKT_SLOT_B,
	PKT_SLOT_MAX
};

/**
 * @brief This structure is in charge to store a packet in memory space
 */
typedef struct _pkt_slot_t {
	packet_t service_header;
	bool free;
} _pkt_slot_t;

_pkt_slot_t pkt_slots[PKT_SLOT_MAX];	//!< Packet send control.

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
	packet->service = MIGRATION_PIPE;
	packet->task_ID = producer_task;
	packet->consumer_task = consumer_task;
	packet->msg_length = size;
}

void pkt_set_data_av(packet_t *packet, int cons_addr, int prod_task, int cons_task, int prod_addr)
{
	packet->header = cons_addr;
	packet->service = DATA_AV;
	packet->producer_task = prod_task;
	packet->consumer_task = cons_task;
	packet->requesting_processor = prod_addr;
}

void pkt_set_message_request(packet_t *packet, int prod_addr, int cons_addr, int prod_task, int cons_task)
{
	packet->header = prod_addr;
	packet->service = MESSAGE_REQUEST;
	packet->requesting_processor = cons_addr;
	packet->producer_task = prod_task;
	packet->consumer_task = cons_task;
}

void pkt_set_migration_code(packet_t *packet, int addr, int task, size_t text_size, int mapper_task, int mapper_addr)
{
	packet->header = addr;
	packet->service = MIGRATION_CODE;
	packet->task_ID = task;
	packet->code_size = text_size;
	packet->mapper_task = mapper_task;
	packet->mapper_address = mapper_addr;
}

void pkt_set_migration_tcb(packet_t *packet, int addr, int id, void* pc, unsigned received)
{
	packet->header = addr;
	packet->service = MIGRATION_TCB;
	packet->task_ID = id;
	packet->program_counter = (unsigned)pc;
	packet->waiting_msg = received;
}

void pkt_set_migration_tl(packet_t *packet, int addr, unsigned service, int id, size_t size)
{
	packet->header = addr;
	packet->service = service;
	packet->task_ID = id;
	packet->request_size = size;
}

void pkt_set_migration_data_bss(packet_t *packet, int addr, int id, size_t data_size, size_t bss_size)
{
	packet->header = addr;
	packet->service = MIGRATION_DATA_BSS;
	packet->task_ID = id;
	packet->data_size = data_size;
	packet->bss_size = bss_size;
}

void pkt_set_migration_heap(packet_t *packet, int addr, int id, size_t size)
{
	packet->header = addr;
	packet->service = MIGRATION_HEAP;
	packet->task_ID = id;
	packet->heap_size = size;
}

void pkt_set_migration_stack(packet_t *packet, int addr, int id, size_t stack_size)
{
	packet->header = addr;
	packet->service = MIGRATION_STACK;
	packet->task_ID = id;
	packet->stack_size = stack_size;
}

void pkt_set_migration_app(packet_t *packet, int addr, int id, size_t task_cnt)
{
	packet->header = addr;
	packet->service = MIGRATION_TASK_LOCATION;
	packet->task_ID = id;
	packet->request_size = task_cnt;
}

void pkt_set_migration_sched(
	packet_t *packet, 
	int addr, 
	int id, 
	unsigned period, 
	unsigned deadline, 
	unsigned waiting_msg, 
	unsigned exec_time
)
{
	packet->header = addr;
	packet->service = MIGRATION_SCHED;
	packet->task_ID = id;
	/* RT constraints */
	packet->period = period;
	packet->deadline = deadline;
	packet->waiting_msg = waiting_msg;
	packet->execution_time = exec_time;
}

void pkt_set_dmni_info(packet_t *packet, size_t payload_size)
{
	packet->payload_size = (PKT_SIZE - 2) + payload_size;
	packet->transaction = 0;
	packet->source_PE = MMR_NI_CONFIG;
	packet->timestamp = MMR_TICK_COUNTER;
}
