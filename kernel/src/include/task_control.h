/**
 * MA-Memphis
 * @file task_control.h
 *
 * @author Marcelo Ruaro (marcelo.ruaro@acad.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date June 2016
 * 
 * @brief This module declares the task control block (TCB) functions.
 * 
 * @details This structure stores information of the user's tasks that are 
 * running.
 */

#pragma once

#include "ipipe.h"
#include "opipe.h"
#include "message_request.h"
#include "data_available.h"
#include "task_scheduler.h"
#include "mmr.h"
#include "hal.h"

/** @brief This structure stores information of the running tasks */
typedef struct _tcb {
	unsigned int registers[HAL_MAX_REGISTERS];	//!< Register bank
	unsigned int pc;							//!< Register file
	void *offset;								//!< Initial address of the task code in page

	int id;							//!< TCB identifier
	unsigned int text_length;		//!< Memory TEXT section lenght in bytes
	unsigned int data_length;		//!< Memory DATA section lenght in bytes
	unsigned int bss_length;		//!< Memory BSS section lenght in bytes
	unsigned int heap_end;

	int mapper_address;
	int mapper_task;

	int proc_to_migrate;	//!< Processor to migrate the task

	int task_location[PKG_MAX_TASKS_APP];	//!< Location of app tasks

	opipe_t *pipe_out;							//!< Temporary buffer for outbound messages.
	ipipe_t *pipe_in;							//!< Pointer storage for inbound messages.
	message_request_t message_request[MR_MAX];	//!< Message request array
	data_av_fifo_t data_av;						//!< Data available fifo

	scheduler_t scheduler;	//!< Scheduling control structure

	bool called_exit;
} tcb_t;

/**
 * @brief Initializes the TCB structures
 */
void tcb_init();

/**
 * @brief Gets the TCB array
 * 
 * @return Pointer to the first element of the TCB array.
 */
tcb_t *tcb_get();

/**
 * @brief Searches for the TCB of a given task ID
 * 
 * @param task The ID of the task.
 * 
 * @return Pointer to the TCB of the task.
 */
tcb_t *tcb_search(int task);

/**
 * @brief Searches for a free TCB
 * 
 * @return Pointer to the free TCB. NULL case it's full.
 */
tcb_t *tcb_free_get();

/**
 * @brief Clears the TCB to allocate a new task
 * 
 * @param tcb Pointer to the TCB
 * @param id ID of the task
 * @param code_sz Size of the code section
 * @param data_sz Size of the data section
 * @param bss_sz Size of the BSS section
 * @param entry_point Starting execution address
 * @param mapper_task ID of the mapper task
 * @param mapper_addr Address of the mapper task
 */
void tcb_alloc(
	tcb_t *tcb, 
	int id, 
	unsigned int code_sz, 
	unsigned int data_sz, 
	unsigned int bss_sz, 
	unsigned entry_point, 
	int mapper_task, 
	int mapper_addr
);

/**
 * @brief Clears the TCB to allocate a migrated task
 * 
 * @param tcb Pointer to the TCB
 * @param id ID of the task
 * @param code_sz Size of the code section
 * @param mapper_tak ID of the mapper task
 * @param mapper_addr Address of the mapper task
 */
void tcb_alloc_migrated(tcb_t *tcb, int id, unsigned int code_sz, int mapper_task, int mapper_addr);

/**
 * @brief Gets the pinter to the input pipe
 * 
 * @param tcb Pointer to the TCB
 * 
 * @return ipipe_t* Pointer to the input pipe
 */
ipipe_t *tcb_get_ipipe(tcb_t *tcb);

/**
 * @brief Gets the offset of a task
 * 
 * @param tcb Pointer to the TCB
 * 
 * @return Address of the task offset
 */
void *tcb_get_offset(tcb_t *tcb);

/**
 * @brief Gets the application ID of a running task
 * 
 * @param tcb Pointer to the TCB
 * 
 * @return The ID of the application
 */
int tcb_get_appid(tcb_t *tcb);

/**
 * @brief Identifies if the process need to be migrated
 * 
 * @param tcb Pointer to the TCB
 * 
 * @return True if needs to be migrated, otherwise false.
 */
bool tcb_need_migration(tcb_t *tcb);

/**
 * @brief Gets the address to migrate to
 * 
 * @param tcb Pointer to the TCB
 * 
 * @return The address of the migration target
 */
int tcb_get_migrate_addr(tcb_t *tcb);

/**
 * @brief Sets the migration address target
 * 
 * @param tcb Pointer to the TCB
 * @param addr Address of the target. -1 to disable.
 */
void tcb_set_migrate_addr(tcb_t *tcb, int addr);

/**
 * @brief Gets the program counter of a task
 * 
 * @param tcb Pointer to the TCB
 * 
 * @return Value of the pc
 */
unsigned int tcb_get_pc(tcb_t *tcb);

/**
 * @brief Gets the stack pointer of a task
 * 
 * @param tcb Pointer to the TCB
 * 
 * @return Value of the stack pointer
 */
unsigned int tcb_get_sp(tcb_t *tcb);

/**
 * @brief Gets the id of a task
 * 
 * @param tcb Pointer to the TCB
 * 
 * @return Number of the complete ID
 */
int tcb_get_id(tcb_t *tcb);

/**
 * @brief Gets a register from the task
 * 
 * @param tcb Pointer to the TCB
 * @param idx Index of the register
 * 
 * @return Value of the register
 */
unsigned int tcb_get_reg(tcb_t *tcb, int idx);

/**
 * @brief Gets the pointer to the message request
 * 
 * @param tcb Pointer to the TCB
 * 
 * @return Pointer to the message request array
 */
message_request_t *tcb_get_mr(tcb_t *tcb);

/**
 * @brief Clears the TCB and make it ready to receive a new task
 * 
 * @param tcb Pointer to the TCB
 */
void tcb_clear(tcb_t *tcb);

/**
 * @brief Gets the length of the code/text section
 * 
 * @param tcb Pointer to the TCB
 * 
 * @return Length of the code
 */
unsigned int tcb_get_code_length(tcb_t *tcb);

/**
 * @brief Gets the length of the data section
 * 
 * @param tcb Pointer to the TCB
 * 
 * @return Length of the data
 */
unsigned int tcb_get_data_length(tcb_t *tcb);

/**
 * @brief Sets the data segment length
 * 
 * @param tcb Pointer to the TCB
 * @param data_length Length of the data segment
 */
void tcb_set_data_length(tcb_t *tcb, unsigned data_length);

/**
 * @brief Gets the length of the bss section
 * 
 * @param tcb Pointer to the TCB
 * 
 * @return Length of the bss
 */
unsigned int tcb_get_bss_length(tcb_t *tcb);

/**
 * @brief Sets the bss segment length
 * 
 * @param tcb Pointer to the TCB
 * @param bss_length Length of the bss segment
 */
void tcb_set_bss_length(tcb_t *tcb, unsigned bss_length);

/**
 * @brief Sets the program counter of a task
 * 
 * @details this will take in consideration the offset
 * 
 * @param tcb Pointer to the TCB
 * @param pc Address to write to PC
 */
void tcb_set_pc(tcb_t *tcb, unsigned int pc);

/**
 * @brief Sets the TCB as called exit
 * 
 * @details This will disable migration requests
 * 
 * @param tcb Pointer to the tcb to set
 */
void tcb_set_called_exit(tcb_t *tcb);

/**
 * @brief Checks if the TCB has called exit
 * 
 * @param tcb Pointer to the tcb to set
 * 
 * @return True if called exit
 */
bool tcb_has_called_exit(tcb_t *tcb);

/**
 * @brief Gets the current TCB heap end
 * 
 * @param tcb Pointer to the TCB
 * 
 * @return unsigned heap end byte
 */
unsigned tcb_get_heap_end(tcb_t *tcb);

/**
 * @brief Increments the heap
 * 
 * @param tcb Pointer to the TCB
 * @param addr New heap final address 
 */
void tcb_set_brk(tcb_t *tcb, unsigned incr);

/**
 * @brief Terminates a task after exit
 * 
 * @param tcb Pointer to the TCB to terminate
 */
void tcb_terminate(tcb_t *tcb);

/**
 * @brief Aborts a task
 * 
 * @param tcb Pointer to the TCB to terminate
 */
void tcb_abort_task(tcb_t *tcb);

/**
 * @brief Cleans up a terminated (exited or aborted) task
 * 
 * @param tcb Pointer to the TCB to terminate
 */
void tcb_cleanup(tcb_t *tcb);

/**
 * @brief Creates an input pipe structure
 * 
 * @param tcb Pointer to the TCB
 * 
 * @return ipipe_t* Pointer to the created pipe
 */
ipipe_t *tcb_create_ipipe(tcb_t *tcb);

/**
 * @brief Destroys the input pipe
 * 
 * @param tcb Pointer to the TCB
 */
void tcb_destroy_ipipe(tcb_t *tcb);

/**
 * @brief Creates an output pipe structure
 * 
 * @param tcb Pointer to the TCB
 * 
 * @return opipe_t* Pointer to the created pipe
 */
opipe_t *tcb_create_opipe(tcb_t *tcb);

/**
 * @brief Gets the output pipe structure
 * 
 * @param tcb Pointer to the TCB
 * 
 * @return opipe_t* Pointer to the PIPE. NULL if not present.
 */
opipe_t *tcb_get_opipe(tcb_t *tcb);

/**
 * @brief Destroys the output pipe
 * 
 * @details The message buffer is not freed here. Check DMNI functions.
 * 
 * @param tcb Pointer to the TCB
 */
void tcb_destroy_opipe(tcb_t *tcb);
