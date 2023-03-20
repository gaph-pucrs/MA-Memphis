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

#include <mutils/list.h>

#include "hal.h"
#include "paging.h"
#include "task_location.h"
#include "application.h"
#include "task_scheduler.h"
#include "ipipe.h"
#include "opipe.h"

/** @brief This structure stores information of the running tasks */
typedef struct _tcb {
	unsigned registers[HAL_MAX_REGISTERS];	//!< Register bank
	void *pc;								//!< Program counter content
	page_t *page;							//!< Pointer to the page offset control structure
	
	int id;					//!< TCB identifier
	size_t text_size;		//!< Memory TEXT section size in bytes
	size_t data_size;		//!< Memory DATA section size in bytes
	size_t bss_size;		//!< Memory BSS section size in bytes
	int proc_to_migrate;	//!< Address of the processor to migrate

	void *heap_end;							//!< Address of the heap section end

	tl_t mapper;
	list_t message_requests;	//!< List of message requests
	list_t data_avs;			//!< List of data available messages

	app_t *app;				//!< Pointer to the app structure containing task location
	sched_t *scheduler;	//!< Pointer to the scheduling control structure
	ipipe_t *pipe_in;		//!< Pointer storage for inbound messages
	opipe_t *pipe_out;		//!< Temporary buffer for outbound messages

	bool called_exit;		//!< Flags that the task has exited
} tcb_t;

/**
 * @brief Initializes the TCB structures
 */
void tcb_init();

/**
 * @brief Pushes a TCB to the list
 * 
 * @param tcb Pointer to the TCB
 * @return list_entry_t* Pointer to the list entry
 */
list_entry_t *tcb_push_back(tcb_t *tcb);

/**
 * @brief Finds a TCB
 * 
 * @param task ID of the task
 * @return tcb_t* Pointer to a TCB
 */
tcb_t *tcb_find(int task);

/**
 * @brief Clears the TCB to allocate a new task
 * 
 * @param tcb Pointer to the TCB
 * @param id ID of the task
 * @param text_size Size of the code section
 * @param data_size Size of the data section
 * @param bss_size Size of the BSS section
 * @param mapper_task ID of the mapper task
 * @param mapper_addr Address of the mapper task
 * @param entry_point Starting execution address
 */
void tcb_alloc(
	tcb_t *tcb, 
	int id, 
	size_t text_size, 
	size_t data_size, 
	size_t bss_size, 
	int mapper_task, 
	int mapper_addr, 
	void *entry_point
);

/**
 * @brief Verifies the stack of a task
 * 
 * @param tcb Pointer to the TCB
 * @return true If the stack is OK
 * @return false If the stack is colliding with heap
 */
bool tcb_check_stack(tcb_t *tcb);

/**
 * @brief Aborts a task
 * 
 * @param tcb Pointer to the TCB to terminate
 */
void tcb_abort_task(tcb_t *tcb);

/**
 * @brief Removes a TCB
 * 
 * @param tcb Pointer to the TCB
 */
void tcb_remove(tcb_t *tcb);

/**
 * @brief Gets the output pipe structure
 * 
 * @param tcb Pointer to the TCB
 * 
 * @return opipe_t* Pointer to the PIPE. NULL if not present.
 */
opipe_t *tcb_get_opipe(tcb_t *tcb);

/**
 * @brief Terminates a task after exit
 * 
 * @param tcb Pointer to the TCB to terminate
 */
void tcb_terminate(tcb_t *tcb);

/**
 * @brief Gets the application of a task
 * 
 * @param tcb Pointer to the TCB
 * @return app_t* Pointer to the task
 */
app_t *tcb_get_app(tcb_t *tcb);

/**
 * @brief Gets the pinter to the input pipe
 * 
 * @param tcb Pointer to the TCB
 * 
 * @return ipipe_t* Pointer to the input pipe
 */
ipipe_t *tcb_get_ipipe(tcb_t *tcb);

/**
 * @brief Identifies if the process need to be migrated
 * 
 * @param tcb Pointer to the TCB
 * 
 * @return True if needs to be migrated, otherwise false.
 */
bool tcb_need_migration(tcb_t *tcb);

/**
 * @brief Creates an output pipe structure
 * 
 * @param tcb Pointer to the TCB
 * 
 * @return opipe_t* Pointer to the created pipe
 */
opipe_t *tcb_create_opipe(tcb_t *tcb);

/**
 * @brief Destroys the output pipe
 * 
 * @details The message buffer is not freed here. Check DMNI functions.
 * 
 * @param tcb Pointer to the TCB
 */
void tcb_destroy_opipe(tcb_t *tcb);

/**
 * @brief Gets the list of message requests
 * 
 * @param tcb Pointer to the TCB
 * @return list_t* Pointer to the list
 */
list_t *tcb_get_msgreqs(tcb_t *tcb);

/**
 * @brief Gets the list of data available
 * 
 * @param tcb Pointer to the TCB
 * @return list_t* Pointer to the list
 */
list_t *tcb_get_davs(tcb_t *tcb);

/**
 * @brief Sends a task allocated message
 * 
 * @param tcb Pointer to the TCB
 * @return true If should schedule
 * @return false If should not schedule
 */
bool tcb_send_allocated(tcb_t *tcb);

/**
 * @brief Gets the offset of a task
 * 
 * @param tcb Pointer to the TCB
 * 
 * @return Address of the task offset
 */
void *tcb_get_offset(tcb_t *tcb);

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
 * @return void* Value of the pc
 */
void *tcb_get_pc(tcb_t *tcb);

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
 * @brief Gets a pointer to the array of registers
 * 
 * @param tcb Pointer to the TCB
 * @return unsigned* Array of registers
 */
unsigned *tcb_get_regs(tcb_t *tcb);

/**
 * @brief Gets the size of the text section
 * 
 * @param tcb Pointer to the TCB
 * 
 * @return Size of the text section
 */
size_t tcb_get_text_size(tcb_t *tcb);

/**
 * @brief Gets the size of the data section
 * 
 * @param tcb Pointer to the TCB
 * 
 * @return Size of the data
 */
size_t tcb_get_data_size(tcb_t *tcb);

/**
 * @brief Sets the data segment length
 * 
 * @param tcb Pointer to the TCB
 * @param data_size Size of the data segment
 */
void tcb_set_data_size(tcb_t *tcb, size_t data_size);

/**
 * @brief Gets the size of the bss section
 * 
 * @param tcb Pointer to the TCB
 * 
 * @return Size of the bss
 */
size_t tcb_get_bss_size(tcb_t *tcb);

/**
 * @brief Sets the bss segment size
 * 
 * @param tcb Pointer to the TCB
 * @param bss_size Size of the bss segment
 */
void tcb_set_bss_size(tcb_t *tcb, size_t bss_size);

/**
 * @brief Sets the program counter of a task
 * 
 * @details this will take in consideration the offset
 * 
 * @param tcb Pointer to the TCB
 * @param pc Address to write to PC
 */
void tcb_set_pc(tcb_t *tcb, void *pc);

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
 * @return void* heap end address
 */
void *tcb_get_heap_end(tcb_t *tcb);

/**
 * @brief Increments the heap
 * 
 * @param tcb Pointer to the TCB
 * @param addr New heap final address 
 */
void tcb_set_brk(tcb_t *tcb, void *addr);

/**
 * @brief Sets the scheduler of a task
 * 
 * @param tcb Pointer to the TCB
 * @param sched Pointer to the scheduler
 */
void tcb_set_sched(tcb_t *tcb, sched_t *sched);

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
 * @brief Gets the scheduler pointer
 * 
 * @param tcb Pointer to the tcb
 * @return sched_t* Pointer to the scheduler
 */
sched_t *tcb_get_sched(tcb_t *tcb);

/**
 * @brief Gets the mapper location
 * 
 * @param tcb Pointer to the TCB
 * @return tl_t* Pointer to the task location
 */
tl_t *tcb_get_mapper(tcb_t *tcb);

/**
 * @brief Sets the return value of a task
 * 
 * @param tcb Pointer to the TCB
 * @param ret Return value
 */
void tcb_set_ret(tcb_t *tcb, int ret);

/**
 * @brief Gets the number of tasks in the PE
 * 
 * @return size_t Number of tasks
 */
size_t tcb_size();

/**
 * @brief Destroy all management task TCBs, leaving only the requester
 * 
 * @param requester TCB of the action requester
 * @return int 0 if success, EFAULT if destroyed a non-management TCB
 */
int tcb_destroy_management(tcb_t *requester);
