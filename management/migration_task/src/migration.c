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

void migration_init(migration_ring_t *migration)
{
	// migration_ring.empty = true;
	// migration_ring.full = false;
	migration->head = 0;
	// migration_ring.tail = 0;
	for(int i = 0; i < PKG_PENDING_SVC_MAX; i++){
		migration->tasks[i].id = -1;
		migration->tasks[i].missed_cnt = 0;
	}
}

void migration_check_rt(migration_ring_t *migration, oda_t *actuator, int id, int remaining)
{
	if(remaining < 0){
		migration_task_t *task = migration_search_task(migration, id);
		if(task == NULL)
			task = migration_task_insert(migration, id);
		
		migration_task_inc_miss(task);
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

migration_task_t *migration_search_task(migration_ring_t *migration, int id)
{
	migration_task_t *task = NULL;
	for(int i = 0; i < PKG_PENDING_SVC_MAX; i++){
		if(migration->tasks[i].id == id){
			task = &migration->tasks[i];
			break;
		}
	}
	return task;
}

migration_task_t *migration_task_insert(migration_ring_t *migration, int id)
{
	migration_task_t *task = &migration->tasks[migration->head++];
	migration->head %= PKG_PENDING_SVC_MAX;

	task->id = id;
	task->missed_cnt = 0;
	return task;
}

void migration_task_inc_miss(migration_task_t *task)
{
	task->missed_cnt++;
}

int migration_task_get_miss(migration_task_t *task)
{
	return task->missed_cnt;
}

void migration_task_clear(migration_task_t *task)
{
	task->missed_cnt = 0;
}
