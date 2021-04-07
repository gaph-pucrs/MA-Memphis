#include <api.h>
#include <stdlib.h>

#include "migration.h"
#include "decider.h"
#include "ma_pkg.h"
#include "tag.h"

migration_ring_t migration_ring;

void migration_init()
{
	// migration_ring.empty = true;
	// migration_ring.full = false;
	migration_ring.head = 0;
	// migration_ring.tail = 0;
	for(int i = 0; i < PKG_PENDING_SVC_MAX; i++){
		migration_ring.tasks[i].id = -1;
		migration_ring.tasks[i].missed_cnt = 0;
	}
}

void migration_test(int id, int remaining)
{
	if(remaining < 0){
		migration_task_t *task = migration_search_task(id);
		if(task == NULL)
			task = migration_task_insert(id);
		
		migration_update_deadline_miss(task);
		if(decider_enabled() && migration_miss_count(task) >= 3){
			migration_clear_miss(task);
			Message msg;
			msg.msg[0] = TASK_MIGRATION_MAP;
			msg.msg[1] = id;
			msg.length = 2;
			Echo("Requesting migration for task "); Echo(itoa(id)); Echo("\n");

			SSend(&msg, decider_get_actor());
		}
	}
}

migration_task_t *migration_search_task(int id)
{
	migration_task_t *task = NULL;
	for(int i = 0; i < PKG_PENDING_SVC_MAX; i++){
		if(migration_ring.tasks[i].id == id){
			task = &migration_ring.tasks[i];
			break;
		}
	}
	return task;
}

migration_task_t *migration_task_insert(int id)
{
	migration_task_t *task = &migration_ring.tasks[migration_ring.head++];
	migration_ring.head %= PKG_PENDING_SVC_MAX;

	task->id = id;
	task->missed_cnt = 0;
	return task;
}

void migration_update_deadline_miss(migration_task_t *task)
{
	task->missed_cnt++;
}

int migration_miss_count(migration_task_t *task)
{
	return task->missed_cnt;
}

void migration_clear_miss(migration_task_t *task)
{
	task->missed_cnt = 0;
}
