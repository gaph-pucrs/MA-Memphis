/**
 * MA-Memphis
 * @file interrupts.h
 * 
 * @author Angelo Elias Dalzotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date August 2020
 * 
 * @brief Declares the interrupts procedures of the kernel.
 */

#pragma once

#include <stdbool.h>
#include <stddef.h>

#include <memphis/monitor.h>

#include "task_control.h"
#include "broadcast.h"

enum IRQ_FLAGS {
	IRQ_PENDING_SERVICE = 0x01,
	IRQ_SLACK_TIME = 0x02,
	IRQ_SCHEDULER = 0x08,
	IRQ_NOC = 0x20,
	IRQ_BRNOC = 0x40
};

/**
 * @brief Function called by the HAL interruption handler.
 * 
 * @details It cannot send a packet when the DMNI is already sending a packet.
 * This function implementation should assure this behavior.
 * 
 * @param status Status of the interruption. Signals the interruption type.
 * 
 * @return Pointer to the scheduled task
 */
tcb_t *isr_isr(unsigned status);

/**
 * @brief Handles an interruption coming from a broadcast message
 * 
 * @param packet Pointer to BrNoC packet
 * 
 * @return True if the scheduler should be called
 */
bool isr_handle_broadcast(bcast_t *packet);

/** 
 * @brief Handles the packet coming from the NoC.
 * 
 * @param packet Pointer to the packet received.
 * 
 * @return True if the scheduler should be called.
 */
bool isr_handle_pkt(volatile packet_t *packet);

/**
 * @brief Receive a message request and add to the proper task TCB
 * 
 * @details This is a message synchronization mechanism
 * 
 * @param cons_task Consumer task ID (sender of the packet)
 * @param cons_addr Consumer address (sender of the packet)
 * @param prod_task Producer task ID (receiver of the packet)
 * 
 * @return True if the scheduler should be called.
 */
bool isr_message_request(int cons_task, int cons_addr, int prod_task);

/**
 * @brief Receives a message from the NoC
 * 
 * @param cons_task ID of the consumer task
 * @param prod_task ID of the producer task
 * @param prod_addr Address of the producer task
 * @param size Size of the message received
 * @param pkt_payload_size Size of the packet payload in flits
 * 
 * @return True if the scheduler should be called
 */
bool isr_message_delivery(int cons_task, int prod_task, int prod_addr, size_t size, unsigned pkt_payload_size);

/**
 * @brief Handles a data available packet
 * 
 * @param cons_task ID of the consumer task (receiver)
 * @param prod_task ID of the producer task
 * @param prod_addr Address of the producer task
 * 
 * @return True if the scheduler should be called.
 */
bool isr_data_available(int cons_task, int prod_task, int prod_addr);

/**
 * @brief Handles a task allocation packet
 * 
 * @param id ID of the new task
 * @param length Code length of the new task
 * @param data_len Length of the data section
 * @param bss_len Length of the BSS section
 * @param entry_point Starting execution address
 * @param mapper_task ID of the mapper task
 * @param mapper_addr Address of the mapper task
 * 
 * @return True if the scheduler should be called
 */
bool isr_task_allocation(
	int id, 
	unsigned length, 
	unsigned data_len, 
	unsigned bss_len, 
	void *entry_point, 
	int mapper_task, 
	int mapper_addr
);

/**
 * @brief Handles a task release packet
 * 
 * @param id ID of the task to release
 * @param task_number Number of the app's tasks
 * @param task_location Pointer to task location array
 * 
 * @return True if the scheduler should be called
 */
bool isr_task_release(int id, int task_number, int *task_location);

/**
 * @brief Handles a task migration order
 * @details This function is called by the source processor (the old processor)
 * 
 * @param id ID of the task to migrate
 * @param addr Address to migrate
 * 
 * @return True if scheduler should be called
 */
bool isr_task_migration(int id, int addr);

/**
 * @brief Handles the code received from migration
 * 
 * @param id ID of the task that has migrated
 * @param code_sz Size of the code
 * @param mapper_task ID of the mapper task
 * @param mapper_addr Address of the mapper task
 * 
 * @return False
 */
bool isr_migration_code(int id, size_t text_size, int mapper_task, int mapper_addr);

/**
 * @brief Handles the TCB received from migration
 * 
 * @param id ID of the task that has migrated
 * @param pc Address of the program counter
 * @param received Number of bytes of the last received message
 *
 * @return False
 */
bool isr_migration_tcb(int id, void *pc, unsigned received);

/**
 * @brief Handles the scheduler received from migration
 * 
 * @param id ID of the task that has migrated
 * @param period Task period
 * @param deadline Task deadline
 * @param exec_time Task execution time
 * @param waiting_msg Task waiting message status
 * @param source Source PE
 * @return true If should scheduled
 * @return false If should not schedule
 */
bool isr_migration_sched(int id, unsigned period, int deadline, unsigned exec_time, unsigned waiting_msg, int source);

/**
 * @brief Handles the task location received from migration (DATA_AV/MESSAGE_REQUEST)
 * 
 * @param id ID of the task that has migrated
 * @param size Number of entries in the task location
 * @param service Service that was received
 * 
 * @return false
 */
bool isr_migration_tl(int id, size_t size, unsigned service);

/**
 * @brief Handles the pipe received from migration
 * 
 * @param id ID of the task that has migrated
 * @param cons_task ID of the consumer task of the message
 * @param size Number of bytes of message
 * 
 * @return False
 */
bool isr_migration_pipe(int id, int cons_task, size_t size);

/**
 * @brief Handles the stack received from migration
 * 
 * @param id ID of the task that has migrated
 * @param size size of the stack
 * 
 * @return False
 */
bool isr_migration_stack(int id, size_t size);

/**
 * @brief Handles the heap received from migration
 * 
 * @param id ID of the task that has migrated
 * @param heap_size Length of the heap
 * 
 * @return False
 */
bool isr_migration_heap(int id, size_t heap_size);

/**
 * @brief Handles the data and bss received from migration
 * 
 * @param id ID of the task that has migrated
 * @param data_size Size of the data section
 * @param bss_size Size of the bss section
 * 
 * @return False
 */
bool isr_migration_data_bss(int id, size_t data_size, size_t bss_size);

/**
 * @brief Handles the application data (task location) migrated
 * 
 * @param id ID of the migrated task
 * @param task_cnt Number of tasks to receive
 * @return false 
 */
bool isr_migration_app(int id, size_t task_cnt);

/**
 * @brief Clears a task from the monitoring DMNI LUT
 * 
 * @param task ID of the task to clear
 * 
 * @return False
 */
bool isr_clear_mon_table(int task);

/**
 * @brief Registers an announced observer
 * 
 * @param type Monitoring type
 * @param addr Address of the observer
 * 
 * @return False
 */
bool isr_announce_mon(enum MONITOR_TYPE type, int addr);

/**
 * @brief Aborts a task
 * 
 * @param id ID of the task to abort
 * 
 * @return True if the current running task is the aborted
 */
bool isr_abort_task(int id);

/**
 * @brief Clears application migration data
 * 
 * @param id ID of the application
 * 
 * @return False
 */
bool isr_app_terminated(int id);

/**
 * @brief Asks to halt the PE
 * 
 * @param task Mapper task ID that requested the halt
 * @param addr Address of the mapper task
 * 
 * @return false 
 */
bool isr_halt_pe(int task, int addr);
