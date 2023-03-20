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

#include "task_location.h"
#include "task_control.h"

/**
 * @brief Initializes the task migration structures
 */
void tm_init();

/**
 * @brief Finds a task migration
 * 
 * @param task ID of the migrated task
 * @return tl_t* Task location structure
 */
tl_t *tm_find(int task);

/**
 * @brief Creates and stores a task migration information
 * 
 * @param task ID of the migrated task
 * @param addr Address where the task has migrated
 * @return tl_t* Pointer to the task location structure created
 */
tl_t *tm_emplace_back(int task, int addr);

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
 * @param id ID of the TCB
 * @param addr Address of the TCB
 */
void tm_send_text(tcb_t *tcb, int id, int addr);

/**
 * @brief Migrates the TCB
 * 
 * @param tcb Pointer to the TCB
 * @param id ID of the migrating task
 * @param addr Target address
 */
void tm_send_tcb(tcb_t *tcb, int id, int addr);

/**
 * @brief Migrates the application task location
 * 
 * @param tcb Pointer to the TCB
 * @param app Pointer to the application
 * @param id ID of the migrating task
 * @param addr Address to migrate
 */
void tm_send_app(tcb_t *tcb, app_t *app, int id, int addr);

/**
 * @brief Migrates a task location for messaging API
 * 
 * @param tcb Pointer to the TCB
 * @param list Pointer to the list to migrate
 * @param service Service to migrate (DATA_AV/MESSAGE_REQUEST)
 * @param id ID of the task
 * @param addr Address to migrate
 */
void tm_send_tl(tcb_t *tcb, list_t *list, unsigned service, int id, int addr);

/**
 * @brief Migrates the output pipe
 * 
 * @param tcb Pointer to the TCB
 * @param id ID of the task
 * @param addr Address to migrate
 */
void tm_send_opipe(tcb_t *tcb, int id, int addr);

/**
 * @brief Migrate the stack
 * 
 * @param tcb Pointer to the TCB
 * @param id ID of the task
 * @param addr Target address
 */
void tm_send_stack(tcb_t *tcb, int id, int addr);

/**
 * @brief Migrate the heap
 * 
 * @param tcb Pointer to the TCB
 * @param id ID of the task
 * @param addr Target address
 */
void tm_send_heap(tcb_t *tcb, int id, int addr);

/**
 * @brief Migrate the data and bss
 * 
 * @param tcb Pointer to the TCB
 * @param id ID of the task
 * @param addr Target address
 */
void tm_send_data_bss(tcb_t *tcb, int id, int addr);

/**
 * @brief Migrates the scheduling information
 * 
 * @param tcb Pointer to the TCB
 * @param id ID of the task
 * @param addr Address to migrate
 */
void tm_send_sched(tcb_t *tcb, int id, int addr);

/**
 * @brief Sends a message to abort a migrating task
 * 
 * @param id ID of the task to abort
 * @param addr Address of the migration destination
 */
void tm_abort_task(int id, int addr);

/**
 * @brief Clears all entries corresponding to an application
 * 
 * @param id Application ID
 */
void tm_clear_app(int id);

/**
 * @brief Checks if the migrated app list is empty
 * 
 * @return true Is empty
 * @return false Not empty
 */
bool tm_empty();
