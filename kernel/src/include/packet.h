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

#include <stdbool.h>
#include <stddef.h>

static const unsigned PKT_SIZE = 13;	//!<Constant Service Header size, based on the structure ServiceHeader. If you change it, please change the same define within app_injector.h

/**
 * @brief This structure is to be filled by the software when needed to send a 
 * packet, or that will be read when the packet is received.
 */
typedef struct _packet {
	unsigned int header;				//!< Target NoC router
	unsigned int payload_size;			//!< Number of flits of the remainer packet
	unsigned int service;				//!< Service being sent. See services.h
	
	union {
		unsigned int producer_task;
		unsigned int task_ID;			
		// unsigned int app_ID;
	};

	union {
	   unsigned int consumer_task;
	   unsigned int mapper_address;
	//    unsigned int hops;
	   unsigned int period;
	};

	unsigned int source_PE;				//!< Packet source PE address
	unsigned int timestamp;				//!< Filled by packet_send()
	unsigned int transaction;

	union {
		unsigned int mapper_task;
		unsigned int msg_length;
		// unsigned int resolution;
		// unsigned int priority;
		unsigned int deadline;
		// unsigned int pkt_latency;
		unsigned int stack_size;
		unsigned int heap_size;
		// unsigned int requesting_task;
		// unsigned int released_proc;
		// unsigned int app_task_number;
		// unsigned int app_descriptor_size;
		unsigned int allocated_processor;
		unsigned int requesting_processor;
	};

	union {
		// unsigned int pkt_size;
		unsigned int waiting_msg;
		unsigned int data_size;
		unsigned int insert_request;
	};

	union {
		unsigned int code_size;
		// unsigned int max_free_procs;
		unsigned int execution_time;
	};

	union {
		unsigned int bss_size;
		unsigned int cpu_slack_time;
		unsigned int request_size;
	};

	union {
		// unsigned int initial_address;
		unsigned int program_counter;
		unsigned int utilization;
	};

} packet_t;

/**
 * @brief This structure is in charge to store a packet in memory space
 */
typedef struct _pkt_slot_t {
	packet_t service_header;
	bool free;
} pkt_slot_t;

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
 * @brief Sets DMNI info into the packet structure
 * 
 * @param packet Pointer of the message structure
 * @param payload_size Payload size in flits (32-bit). 0 if none.
 */
void pkt_set_dmni_info(packet_t *packet, size_t payload_size);
