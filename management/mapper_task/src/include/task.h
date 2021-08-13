/**
 * MA-Memphis
 * @file task.h
 *
 * @author Angelo Elias Dalzotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date March 2021
 * 
 * @brief Task structures for the mapper
 */

#pragma once

#include <stdbool.h>

#include "app.h"

typedef enum _task_status {
	BLOCKED,
	RUNNING,
	MIGRATING
} task_status_t;

/* Forward declaration */
typedef struct _app app_t;

typedef struct _task task_t;

struct _task {
	int id;
	int proc_idx;
	int old_proc;
	unsigned type_tag;
	task_status_t status;

	unsigned pred_cnt;
	task_t *predecessors[PKG_MAX_TASKS_APP - 1];
	unsigned succ_cnt;
	task_t *successors[PKG_MAX_TASKS_APP - 1];
};

/**
 * @brief Initialize the tasks
 * 
 * @param tasks List of tasks
 */
void task_init(task_t *tasks);

/**
 * @brief Gets a free task structure
 * 
 * @param tasks Array of tasks
 * 
 * @return Pointer to a free task structure
 */
task_t *task_get_free(task_t *tasks);

/**
 * @brief Order the successors
 * 
 * @details This function should be called with the initial tasks in the order array
 * 
 * @param order Array of task pointer containing the mapping order
 * @param ordered Pointer to the counter of ordered ctg predecessors
 * @param order_idx Pointer to the counter of tasks in the order array
 * @param task_cnt Total tasks of the app
 */
void task_order_successors(task_t *order[], unsigned *ordered, unsigned *order_idx, int task_cnt);

/**
 * @brief Verifies if a task is in the order array
 * 
 * @param task Pointer to the task to verify
 * @param order Array of task pointers containing the mapping order
 * @param order_cnt Number of tasks already ordered
 *
 * @return True if task is in the array
 */
bool task_is_ordered(task_t *task, task_t *order[], unsigned order_cnt);

/**
 * @brief Terminates a task
 * 
 * @param task Pointer to task to terminate
 * 
 * @return Address of old processor if migration was on the fly. -1 if task wasn't migrating.
 */
int task_terminate(task_t *task);
