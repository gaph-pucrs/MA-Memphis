/**
 * 
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

#include "hal.h"
#include "task_location.h"
#include "pkg.h"
#include "pipe.h"
#include "message_request.h"
#include "data_available.h"
#include "task_scheduler.h"

/** @brief This structure stores information of the running tasks */
typedef struct _tcb {
	hal_word_t registers[HAL_MAX_REGISTERS];	//!< Register bank
	hal_word_t pc;								//!< Register file
	hal_word_t offset;			//!< Initial address of the task code in page

	int id;					//!< TCB identifier
	hal_word_t text_lenght;		//!< Memory TEXT section lenght in bytes
	hal_word_t data_lenght;		//!< Memory DATA section lenght in bytes
	hal_word_t bss_lenght;		//!< Memory BSS section lenght in bytes

	int mapper_address;
	int mapper_task;

	int proc_to_migrate;	//!< Processor to migrate the task

	int task_location[PKG_MAX_TASKS_APP];	//!< Location of app tasks

	pipe_t pipe;											//!< Temporary buffer for outbound messages.
	message_request_t message_request[PKG_MAX_TASKS_APP];	//!< Message request array
	data_av_fifo_t data_av;									//!< Data available fifo

	scheduler_t scheduler;	//!< Scheduling control structure
} tcb_t;

/**
 * @brief Idle function.
 */
void tcb_idle_task();

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
 * @brief Gets the idle task TCB
 * 
 * @return Pointer to the idle task TCB
 */
tcb_t *tcb_get_idle();

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
 * @param mapper_task ID of the mapper task
 * @param mapper_addr Address of the mapper task
 */
void tcb_alloc(tcb_t *tcb, int id, hal_word_t code_sz, int mapper_task, int mapper_addr);

/**
 * @brief Updates the TCB with proper section sizes
 * 
 * @param tcb Pointer to the TCB
 * @param data_sz Size of the section data
 * @param bss_sz Size of the section bss
 */
void tcb_update_sections(tcb_t *tcb, hal_word_t data_sz, hal_word_t bss_sz);

/**
 * @brief Gets the pointer to the message variable
 * 
 * @param tcb Pointer to the TCB
 * 
 * @return Pointer to the message structure
 */
message_t *tcb_get_message(tcb_t *tcb);

/**
 * @brief Gets the offset of a task
 * 
 * @param tcb Pointer to the TCB
 * 
 * @return Address of the task offset
 */
hal_word_t tcb_get_offset(tcb_t *tcb);

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
 * @brief Gets the stack pointer of a task
 * 
 * @param tcb Pointer to the TCB
 * 
 * @return Value of the stack pointer
 */
hal_word_t tcb_get_sp(tcb_t *tcb);

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
hal_word_t tcb_get_reg(tcb_t *tcb, int idx);

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
hal_word_t tcb_get_code_length(tcb_t *tcb);

/**
 * @brief Gets the length of the data section
 * 
 * @param tcb Pointer to the TCB
 * 
 * @return Length of the data
 */
hal_word_t tcb_get_data_length(tcb_t *tcb);

/**
 * @brief Gets the length of the bss section
 * 
 * @param tcb Pointer to the TCB
 * 
 * @return Length of the bss
 */
hal_word_t tcb_get_bss_length(tcb_t *tcb);

/**
 * @brief Sets the program counter of a task
 * 
 * @details this will take in consideration the offset
 * 
 * @param tcb Pointer to the TCB
 * @param pc Address to write to PC
 */
void tcb_set_pc(tcb_t *tcb, hal_word_t pc);

/**
 * @brief Sets the stack pointer of a task
 * 
 * @details This will take in consideration the offser
 * 
 * @param tcb Pointer to the TCB
 * @param sp Value of the stack pointer
 */
void tcb_set_sp(tcb_t *tcb, hal_word_t sp);
