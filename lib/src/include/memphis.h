/**
 * MA-Memphis
 * @file memphis.h
 *
 * @author Angelo Elias Dalzotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date May 2021
 * 
 * @brief Memphis API
 */

#pragma once

#include <stdint.h>

#define MEMPHIS_KERNEL_MSG 0x10000000
#define MEMPHIS_FORCE_PORT 0x80000000
#define PKG_MAX_MSG_SIZE 128

#define BR_SVC_TGT	5
#define BR_SVC_ALL	6

enum SYSCALL {
	EXIT,
	WRITEPIPE,
	READPIPE,
	GETTICK,
	REALTIME = 5,
	GETID = 8,
	GETLOCATION,
	SCALL_BR_SEND_ALL,
	SCALL_MON_PTR,
	SCALL_BR_SEND_TGT
};

typedef struct _message {
	unsigned short length;
	unsigned int payload[PKG_MAX_MSG_SIZE];
} message_t;

extern int system_call();
extern const uint32_t _has_priv;

/**
 * @brief Gets the ID of the running task
 * 
 * @return The complete ID
 */
int memphis_get_id();

/**
 * @brief Gets the PE address of the runnning task
 * 
 * @return The PE address
 */
int memphis_get_addr();

/**
 * @brief Sends a message through Hermes
 * 
 * @param msg Pointer to the message structure
 * @param target_id ID of the consumer task
 * 
 * @return 0
 */
int memphis_send(message_t *msg, int target_id);

/**
 * @brief Sends a message through Hermes
 * 
 * @param msg Pointer to the message structure
 * @param target_id ID of the consumer task
 * 
 * @return 0
 */
int memphis_receive(message_t *msg, int source_id);

/**
 * @brief Gets the current tick count
 * 
 * @return Tick count
 */
int memphis_get_tick();

/**
 * @brief Sends a message with a 3-way handshake
 * 
 * @param msg Pointer to the message structure
 * @param target_id ID of the consumer task
 * 
 * @return 0
 */
int memphis_send_any(message_t *msg, int target_id);

/**
 * @brief Receives a message from any producer
 * 
 * @param msg Pointer to the message structure to store the message
 * 
 * @return 0
 */
int memphis_receive_any(message_t *msg);

/**
 * @brief Set the running task as real-time
 * 
 * @details
 * Deadline should be less than period
 * Execution time should be less than deadline
 * % Load = exec_time/period * 100
 * Execution time should be discovered via profiling
 * 
 * @param period Task period in ticks
 * @param deadline Task deadline in ticks
 * @param exec_time Task execution time in ticks
 * 
 * @return 0
 */
int memphis_real_time(int period, int deadline, int exec_time);

/**
 * @brief Sends message via broadcast ALL
 * 
 * @param payload Message to send (1 word)
 * @param ksvc Message Kernel Service, used for TGT and ALL (check services.h)
 * 
 * @return 1
 */
int memphis_br_send_all(uint32_t payload, uint8_t ksvc);

/**
 * @brief Sends message via broadcast TARGET
 * 
 * @param payload Message to send (1 word)
 * @param target Target address to send
 * @param ksvc Message Kernel Service, used for TGT and ALL (check services.h)
 * 
 * @return 1
 */
int memphis_br_send_tgt(uint32_t payload, uint16_t target, uint8_t ksvc);
