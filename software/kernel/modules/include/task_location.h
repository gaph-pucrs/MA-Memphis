/**
 * 
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
 * @brief Sends a message updating the location of a task
 * 
 * @details Assembles and send an UPDATE_TASK_LOCATION.
 * 
 * @param dest_task	ID of the task that needs to know about the update.
 * @param dest_addr Address of the target that needs to know about the update.
 * @param updt_task ID of the task that was migrated.
 * @param updt_addr Address of the task that was migrated.
 */
void tl_send_update(int dest_task, int dest_addr, int updt_task, int updt_addr);

/** 
 * @brief Sends a TASK_ALLOCATED to the mapper
 * 
 * @param allocated_task Pointer to the TCB of the allocated task
 */
void tl_send_allocated(tcb_t *allocated_task);

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
 */
void tl_send_terminated(tcb_t *tcb);

/**
 * @brief Gets the location of a task
 * 
 * @param tcb Searcher TCB
 * @param task Searched task ID
 * 
 * @return Address of the task, or -1 case not found
 */
int tl_search(tcb_t *tcb, int task);

// void init_task_location();

// int get_task_location(int);

// void add_task_location(int, int);

// int remove_task_location(int);

// void clear_app_tasks_locations(int);
