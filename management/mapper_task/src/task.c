/**
 * MA-Memphis
 * @file task.c
 *
 * @author Angelo Elias Dalzotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date March 2021
 * 
 * @brief Task structures for the mapper
 */

#include <stddef.h>

#include "task.h"

task_t *task_get_free(task_t *tasks)
{
	for(int i = 0; i < PKG_MAX_LOCAL_TASKS*PKG_N_PE; i++)
		if(tasks[i].id == -1)
			return &tasks[i];

	return NULL;
}

void task_init(task_t *tasks)
{
	for(int i = 0; i < PKG_MAX_LOCAL_TASKS*PKG_N_PE; i++){
		tasks[i].id = -1;
		tasks[i].pred_cnt = 0;
		tasks[i].succ_cnt = 0;
		for(int j = 0; j < PKG_MAX_TASKS_APP - 1; j++){
			tasks[i].predecessors[j] = NULL;
			tasks[i].successors[j] = NULL;
		}
	}
}

void task_order_successors(task_t *order[], unsigned *ordered, unsigned *order_idx, int task_cnt)
{
	while(*ordered < *order_idx){
		task_t *predecessor = order[*ordered];
		for(int i = 0; i < predecessor->succ_cnt; i++){
			/* Check if successor is not ordered yet */
			task_t *successor = predecessor->successors[i];
			if(!task_is_ordered(successor, order, *order_idx))
				order[(*order_idx)++] = successor;
		}
		(*ordered)++;
	}
}

bool task_is_ordered(task_t *task, task_t *order[], unsigned order_cnt)
{
	bool task_ordered = false;
	for(int i = 0; i < order_cnt; i++){
		if(order[i] == task){
			task_ordered = true;
			break;
		}
	}
	return task_ordered;
}

int task_terminate(task_t *task)
{
	task->id = -1;

	/* Deallocate successors */
	for(int i = 0; i < task->succ_cnt; i++)
		task->successors[i] = NULL;

	task->succ_cnt = 0;
	
	for(int i = 0; i < task->pred_cnt; i++)
		task->predecessors[i] = NULL;

	task->pred_cnt = 0;

	if(task->status == MIGRATING){
		/* The task finished with a migration request on the fly */
		return task->old_proc;
	}
	
	return -1;
}
