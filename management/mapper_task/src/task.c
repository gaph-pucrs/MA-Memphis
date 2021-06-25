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
