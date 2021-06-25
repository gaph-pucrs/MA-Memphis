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
		for(int j = 0; j < PKG_MAX_TASKS_APP - 1; j++)
			tasks[i].consumers[j] = NULL;
	}
}

void task_order_consumers(task_t *order[], unsigned *ordered, unsigned *order_idx, int task_cnt)
{
	while(ordered < order_idx){
		task_t *producer = order[*ordered];
		for(int i = 0; i < task_cnt - 1 && producer->consumers[i] != NULL; i++){
			/* Check if consumer is not ordered yet */
			task_t *consumer = producer->consumers[i];
			if(!task_is_ordered(consumer, order, *order_idx))
				order[(*order_idx)++] = consumer;
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

unsigned task_get_predecessors(task_t *task, app_t *app, task_t *predecessors[])
{
	unsigned cnt = 0;

	int id = task->id & 0xFF;
	for(int t = 0; t < app->task_cnt; t++){
		if(t == id)		/* Ignore same task */
			continue;

		task_t *producer = app->task[t];
		for(int j = 0; j < app->task_cnt - 1 && producer->consumers[j] != NULL; j++){
			task_t *consumer = producer->consumers[j];
			if(consumer == task){
				/* Task 'producer' is producer of task 'task' */
				predecessors[cnt++] = producer;
				break;
			}
		}
	}

	return cnt;
}

int task_terminate(task_t *task, unsigned max_consumers)
{
	task->id = -1;

	/* Deallocate consumers */
	for(int i = 0; i < max_consumers && task->consumers[i] != NULL; i++)
		task->consumers[i] = NULL;

	if(task->status == MIGRATING){
		/* The task finished with a migration request on the fly */
		return task->old_proc;
	}
	
	return -1;
}
