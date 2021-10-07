/**
 * MA-Memphis
 * @file app.c
 *
 * @author Angelo Elias Dalzotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date March 2021
 * 
 * @brief Application structures for the mapper
 */

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

#include "app.h"
#include "mapper.h"

void app_init(app_t *apps)
{
	for(int i = 0; i < PKG_MAX_LOCAL_TASKS*PKG_N_PE; i++){
		apps[i].id = -1;
		for(int j = 0; j < PKG_MAX_TASKS_APP; j++)
			apps[i].task[j] = NULL;
	}
}

app_t *app_get_free(app_t *apps)
{
	for(int i = 0; i < PKG_MAX_LOCAL_TASKS*PKG_N_PE; i++)
		if(apps[i].id == -1)
			return &apps[i];

	return NULL;
}

void app_build(app_t *app, int id, unsigned task_cnt, int *descriptor, int *communication, task_t *tasks, processor_t *processors)
{
	app->id = id;
	app->task_cnt = task_cnt;
	app->allocated_cnt = 0;

	for(int i = 0; i < app->task_cnt; i++){
		app->task[i] = task_get_free(tasks);

		app->task[i]->id = id << 8 | i;
		// printf("Task ID: %d\n", app->task[i]->id);

		int proc_idx = descriptor[i*TASK_DESCRIPTOR_SIZE];
		// printf("Processor address: %d\n", proc_idx);
		if(proc_idx != -1){
			proc_idx = (proc_idx >> 8) + (proc_idx & 0xFF)*PKG_N_PE_X;
			// printf("Processor index: %d\n", proc_idx);
			app->task[i]->processor = &(processors[proc_idx]);
		}

		app->task[i]->type_tag = descriptor[i*TASK_DESCRIPTOR_SIZE + 1];
		// printf("Task type tag: %d\n", app->task[i]->type_tag);

		app->task[i]->status = BLOCKED;
	}

	int comm_i = 0;
	for(int i = 0; i < app->task_cnt; i++){
		int encoded_successor;
		task_t *vertex = app->task[i];
		do {
			encoded_successor = communication[comm_i++];
			int successor_id = abs(encoded_successor) - 1;

			if(successor_id >= 0){
				task_t *successor_task = app->task[successor_id];
				vertex->successors[vertex->succ_cnt++] = successor_task;			/* Add successor to task */
				successor_task->predecessors[successor_task->pred_cnt++] = vertex; 	/* Add task to predecessor */
			}
		} while(encoded_successor > 0);
	}

	// for(int i = 0; i < app->task_cnt; i++){
	// 	printf("Task %d successors: ", id << 8 | i);
	// 	for(int j = 0; j < app->task[i]->succ_cnt; j++){
	// 		printf("%d ", app->task[i]->successors[j]->id);
	// 	}
	// 	puts("\n");
	// }
}

app_t *app_search(app_t *apps, int appid)
{
	for(int i = 0; i < PKG_MAX_TASKS_APP; i++){
		if(apps[i].id == appid)
			return &apps[i];
	}
	return NULL;
}

void app_get_order(app_t *app, task_t *order[])
{
	task_t *initials[app->task_cnt];
	int initial_idx = 0;

	/* For all tasks */
	for(int i = 0; i < app->task_cnt; i++){
		task_t *task = app->task[i];
		if(task->pred_cnt == 0){
			/* Task has no predecessor, thus is initial */
			initials[initial_idx++] = task;
		}
	}

	// for(int i = 0; i < initial_idx; i++)
	// 	printf("Initial %d: %d\n", i, initials[i]->id);

	unsigned ordered = 0;
	unsigned order_idx = 0;

	/* The mapping order starts in each initial task */
	for(int i = 0; i < initial_idx; i++){
		order[order_idx++] = initials[i];
	
		/* Map all immediate successors of the initial task and keep mapping its sucessors */
		task_order_successors(order, &ordered, &order_idx);
	}

	/* One or more cyclic dependencies */
	if(order_idx < app->task_cnt){
		for(int i = 0; i < app->task_cnt; i++){
			if(!task_is_ordered(app->task[i], order, order_idx)){
				/* Treat first non-mapped task as initial */
				order[order_idx++] = app->task[i];
				task_order_successors(order, &ordered, &order_idx);
			}
		}
	}
}
