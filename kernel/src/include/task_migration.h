/**
 * MA-Memphis
 * @file task_migration.h
 *
 * @author Marcelo Ruaro (marcelo.ruaro@acad.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date June 2016
 * 
 * @brief Declares the task migration functions
 */

#pragma once

#include "task_control.h"

typedef struct _migrated_task {
	int task;
	int addr;
} migrated_task_t;

/**
 * @brief Initializes the task migration structures
 */
void tm_init();

/**
 * @brief Add a task to the migrated list
 * 
 * @param task ID of the migrated task
 * @param addr Target address of the migration
 */
void tm_add(int task, int addr);

/**
 * @brief Get the address of a migrated task
 * 
 * @param task ID of the migrated task
 * 
 * @return Address of the migration.
 */
int tm_get_migrated_addr(int task);

/**
 * @brief Migrates a task dynamic memory
 * 
 * @param tcb Pointer to the TCB
 */
void tm_migrate(tcb_t *tcb);

/**
 * @brief Migrates the code section of the task
 * 
 * @param tcb Pointer to the TCB
 */
void tm_send_code(tcb_t *tcb);

/**
 * @brief Migrates the TCB
 * 
 * @param tcb Pointer to the TCB
 * @param addr Target address
 */
void tm_send_tcb(tcb_t *tcb, int addr);

/**
 * @brief Migrates the task location
 * 
 * @param tcb Pointer to the TCB
 * @param addr Target address
 */
void tm_send_tl(tcb_t *tcb, int addr);

/**
 * @brief Migrates the message request
 * 
 * @param tcb Pointer to the TCB
 * @param addr Target address
 */
void tm_send_mr(tcb_t *tcb, int addr);

/**
 * @brief Migrate the data available fifo
 * 
 * @param tcb Pointer to the TCB
 * @param addr Target address
 */
void tm_send_data_av(tcb_t *tcb, int addr);

/**
 * @brief Migrate the pipe
 * 
 * @param tcb Pointer to the TCB
 * @param addr Target address
 */
void tm_send_pipe(tcb_t *tcb, int addr);

/**
 * @brief Migrate the stack
 * 
 * @param tcb Pointer to the TCB
 * @param addr Target address
 */
void tm_send_stack(tcb_t *tcb, int addr);

/**
 * @brief Migrate the data and bss
 * 
 * @param tcb Pointer to the TCB
 * @param addr Target address
 */
void tm_send_data_bss(tcb_t *tcb, int addr);

/**
 * @brief Sends a message to abort a migrating task
 * 
 * @param id ID of the task to abort
 * @param addr Address of the migration destination
 */
void tm_abort_task(int id, int addr);