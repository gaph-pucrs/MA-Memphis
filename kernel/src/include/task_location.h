/**
 * MA-Memphis
 * @file task_location.h
 *
 * @author Marcelo Ruaro (marcelo.ruaro@acad.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date June 2016
 * 
 * @brief Identify where other tasks are allocated.
 */

#pragma once

#include "task_control.h"

/**
 * @brief Initialize the app's tasks locations structure.
 * 
 * @param tcb Pointer to the TCB that will be initialized
 */
void tl_init(tcb_t *tcb);

/** 
 * @brief Sends a TASK_ALLOCATED to the mapper
 * 
 * @param allocated_task Pointer to the TCB of the allocated task
 * 
 * @return True if should schedule
 */
bool tl_send_allocated(tcb_t *allocated_task);

/**
 * @brief Updates a single task location
 * 
 * @param tcb Pointer to the TCB
 * @param id ID of the task to update
 * @param addr Location of the task to update
 */
void tl_insert_update(tcb_t *tcb, int id, int addr);

/** 
 * @brief Sends a task terminated packet to the mapper
 * 
 * @param tcb Pointer of the terminated TCB
 * 
 * @return True if should schedule
 */
bool tl_send_terminated(tcb_t *tcb);

/**
 * @brief Gets the location of a task
 * 
 * @param tcb Searcher TCB
 * @param task Searched task ID
 * 
 * @return Address of the task, or -1 case not found
 */
int tl_search(tcb_t *tcb, int task);

/**
 * @brief Gets the number of tasks of the app
 * 
 * @param tcb Pointer to the TCB
 * 
 * @return Number of tasks in the task location array
 */
unsigned int tl_get_len(tcb_t *tcb);

/**
 * @brief Gets the pointer to the task location array
 * 
 * @param tcb Pointer to the TCB
 * 
 * @return Address of the task location array
 */
int *tl_get_ptr(tcb_t *tcb);

/**
 * @brief Update all local tasks of an app with a new location of a task
 * 
 * @param id Complete ID of the task that has migrated
 * @param addr New address of the application
 */
void tl_update_local(int id, int addr);
