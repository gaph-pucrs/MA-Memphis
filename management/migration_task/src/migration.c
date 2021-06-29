/**
 * MA-Memphis
 * @file migration.c
 *
 * @author Angelo Elias Dalzotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date March 2021
 * 
 * @brief Migration decider functions
 */

#include <stddef.h>
#include <stdio.h>
#include <memphis.h>

#include "migration.h"
#include "services.h"

void migration_init(migration_task_t *tasks)
{
	for(int i = 0; i < PKG_PENDING_SVC_MAX; i++)
		tasks[i].id = -1;
}

void migration_check_rt(migration_task_t *tasks, oda_t *actuator, int id, int remaining)
{
	if(remaining < 0){
		migration_task_t *task = migration_search_task(tasks, id);
		if(task == NULL)
			task = migration_task_insert(tasks, id);
		
		migration_task_inc_miss(tasks, task);
		if(oda_is_enabled(actuator) && migration_task_get_miss(task) >= 3){
			migration_task_clear(task);
			message_t msg;
			msg.payload[0] = TASK_MIGRATION_MAP;
			msg.payload[1] = id;
			msg.length = 2;
			printf("Requesting migration for task %d\n", id);

			memphis_send_any(&msg, actuator->id);
		}
	}
}

migration_task_t *migration_search_task(migration_task_t *tasks, int id)
{
	migration_task_t *task = NULL;
	for(int i = 0; i < PKG_PENDING_SVC_MAX; i++){
		if(tasks[i].id == id){
			task = &tasks[i];
			break;
		}
	}
	return task;
}

migration_task_t *migration_task_insert(migration_task_t *tasks, int id)
{
	migration_task_t *task = NULL;

	for(int i = 0; i < PKG_PENDING_SVC_MAX; i++){
		if(tasks[i].id == -1){
			task = &tasks[i];
			break;
		} else if(task == NULL || task[i].lru_cnt > task->lru_cnt){
			task = &tasks[i];
		}
	}

	task->id = id;
	task->missed_cnt = 0;

	return task;
}

void migration_task_inc_miss(migration_task_t *tasks, migration_task_t *task)
{
	task->missed_cnt++;

	for(int i = 0; i < PKG_PENDING_SVC_MAX; i++){
		if(task[i].id != -1)
			task[i].lru_cnt++;
	}

	task->lru_cnt = 1;
}

int migration_task_get_miss(migration_task_t *task)
{
	return task->missed_cnt;
}

void migration_task_clear(migration_task_t *task)
{
	task->missed_cnt = 0;
}
