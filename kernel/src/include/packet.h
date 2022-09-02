/**
 * MA-Memphis
 * @file packet.h
 *
 * @author Marcelo Ruaro (marcelo.ruaro@acad.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date June 2016
 *
 * @brief This module declares the Packet structure. 
 * 
 * @details This structure is used by all software components to send and 
 * receive packets. It defines the service header of each packets.
 */

#pragma once

#include <stddef.h>

static const unsigned PKT_SIZE = 13;	//!<Constant Service Header size, based on the structure ServiceHeader. If you change it, please change the same define within app_injector.h

/**
 * @brief This structure is to be filled by the software when needed to send a 
 * packet, or that will be read when the packet is received.
 */
typedef struct _packet {
	unsigned header;				//!< Target NoC router
	unsigned payload_size;			//!< Number of flits of the remainer packet
	unsigned service;				//!< Service being sent. See services.h
	
	union {
		unsigned producer_task;
		unsigned task_ID;
	};

	union {
	   unsigned consumer_task;
	   unsigned mapper_address;
	   unsigned period;
	};

	unsigned source_PE;				//!< Packet source PE address
	unsigned timestamp;				//!< Filled by packet_send()
	unsigned transaction;

	union {
		unsigned mapper_task;
		unsigned msg_length;
		unsigned deadline;
		unsigned stack_size;
		unsigned heap_size;
		unsigned allocated_processor;
		unsigned requesting_processor;
	};

	union {
		unsigned waiting_msg;
		unsigned data_size;
		unsigned insert_request;
	};

	union {
		unsigned code_size;
		unsigned execution_time;
	};

	union {
		unsigned bss_size;
		unsigned request_size;
	};

	union {
		unsigned program_counter;
	};

} packet_t;

/**
 * @brief Initializes the service header slots.
 */
void pkt_init();

/**
 * @brief Searches for a free packet slot.
 * 
 * @details A free slot is the one which is not being used by DMNI. Prevents 
 * packet going out of scope before being transmitted by the DMNI.
 * 
 * @return A pointer to a free packet slot.
 */
packet_t *pkt_slot_get();

/**
 * @brief Sets a packet to a message delivery format
 * 
 * @param packet Pointer to the packet structure
 * @param consumer_addr Address of the consumer task
 * @param producer_task ID of the producer task
 * @param consumer_task ID of the consumer task
 * @param size Message size in bytes
 */
void pkt_set_message_delivery(packet_t *packet, int consumer_addr, int producer_task, int consumer_task, size_t size);

/**
 * @brief Sets a packet to a migration pipe format
 * 
 * @param packet Pointer to the packet structure
 * @param addr Address of the migration target
 * @param producer_task ID of the producer task
 * @param consumer_task ID of the consumer task
 * @param size Message size in bytes
 */
void pkt_set_migration_pipe(packet_t *packet, int addr, int producer_task, int consumer_task, int size);

/**
 * @brief Sets a packet to a data available format
 * 
 * @param packet Pointer to the packet structure
 * @param cons_addr Address of the consumer task
 * @param prod_task ID of the producer task
 * @param cons_task ID of the consumer task
 * @param prod_addr Address of the producer task
 */
void pkt_set_data_av(packet_t *packet, int cons_addr, int prod_task, int cons_task, int prod_addr);

/**
 * @brief Sets a packet to a message request format
 * 
 * @param packet Pointer to the packet structure
 * @param prod_addr Address of the producer task
 * @param cons_addr Address of the consumer task
 * @param prod_task ID of the producer task
 * @param cons_task ID of the consumer task
 */
void pkt_set_message_request(packet_t *packet, int prod_addr, int cons_addr, int prod_task, int cons_task);

/**
 * @brief Sets a packet to a migration code format
 * 
 * @param packet Pointer to the packet structure
 * @param addr Migration address
 * @param task ID of the migrated task
 * @param text_size Size of the .text section
 * @param mapper_task ID of the mapper task
 * @param mapper_addr Address of the mapper task
 */
void pkt_set_migration_code(packet_t *packet, int addr, int task, size_t text_size, int mapper_task, int mapper_addr);

/**
 * @brief Sets a packet to a migration TCB format
 * 
 * @param packet Pointer to the packet structure
 * @param addr Migration address
 * @param id ID of the migrated task
 * @param pc Current PC of the migrated task
 * @param received Number of bytes received in the last message
 */
void pkt_set_migration_tcb(packet_t *packet, int addr, int id, void* pc, unsigned received);

/**
 * @brief Sets a packet to a migration task location format
 * 
 * @param packet Pointer to the packet structure
 * @param addr Migration address
 * @param service Service of the migrated structure (MIGRATION_DATA_AV/MIGRATION_MESSAGE_REQUEST)
 * @param id ID of the migrated task
 * @param size Number of elements in the TL to migrate
 */
void pkt_set_migration_tl(packet_t *packet, int addr, unsigned service, int id, size_t size);

/**
 * @brief Sets a packet to a migration data/bss format
 * 
 * @param packet Pointer to the packet structure
 * @param addr Migration address
 * @param id ID of the migrated task
 * @param data_size Number of bytes in .data section
 * @param bss_size Number of bytes in .bss section
 */
void pkt_set_migration_data_bss(packet_t *packet, int addr, int id, size_t data_size, size_t bss_size);

/**
 * @brief Sets a packet to a migration heap format
 * 
 * @param packet Pointer to the packet structure
 * @param addr Migration address
 * @param id ID of the migrated task
 * @param size Number of bytes in heap section 
 */
void pkt_set_migration_heap(packet_t *packet, int addr, int id, size_t size);

/**
 * @brief Sets a packet to a migration stack format
 * 
 * @param packet Pointer to the packet structure
 * @param addr Migration address
 * @param id ID of the migrated task
 * @param stack_size Number of bytes in stack section
 */
void pkt_set_migration_stack(packet_t *packet, int addr, int id, size_t stack_size);

/**
 * @brief Sets a packet to a migration app format
 * 
 * @param packet Pointer to the packet structure
 * @param addr Migration address
 * @param id ID of the migrated task
 * @param task_cnt Number of tasks in application
 */
void pkt_set_migration_app(packet_t *packet, int addr, int id, size_t task_cnt);

/**
 * @brief Sets a packet to a migration scheduler format
 * 
 * @param packet Pointer to the packet structure
 * @param addr Migration address
 * @param id ID of the migrated task
 * @param period Period of the scheduler
 * @param deadline Deadline of the scheduler
 * @param waiting_msg Waiting message state of the scheduler
 * @param exec_time Execution time of the scheduler
 */
void pkt_set_migration_sched(
	packet_t *packet, 
	int addr, 
	int id, 
	unsigned period, 
	unsigned deadline, 
	unsigned waiting_msg, 
	unsigned exec_time
);

/**
 * @brief Sets DMNI info into the packet structure
 * 
 * @param packet Pointer of the message structure
 * @param payload_size Payload size in flits (32-bit). 0 if none.
 */
void pkt_set_dmni_info(packet_t *packet, size_t payload_size);
