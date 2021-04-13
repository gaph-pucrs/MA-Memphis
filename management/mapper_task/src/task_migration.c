#include <stddef.h>

#include <api.h>
#include <stdlib.h>

#include "task_migration.h"
#include "app.h"

void tm_migrate(mapper_t *mapper, int task_id)
{
	Echo("Received migration request to task id "); Echo(itoa(task_id)); Echo(" at time "); Echo(itoa(GetTick())); Echo("\n");

	app_t *app = app_search(mapper->apps, task_id >> 8);
	if(app == NULL){
		Echo("Not found app to migrate with id "); Echo(itoa(task_id >> 8)); Echo("\n");
		return;
	}

	task_t *task = app->task[task_id & 0xFF];
	if(task->id != task_id){
		Echo("Not found task to migrate with id "); Echo(itoa(task_id)); Echo("\n");
		return;
	}
	if(task->status != RUNNING){
		Echo("Will not migrate. Task is either already migrating or waiting for task release\n");
		return;
	}

	task->old_proc = task->proc_idx;
	// unsigned then = GetTick();
	task->proc_idx = processors_get_first_most_free(mapper->processors, task->old_proc);
	// unsigned now = GetTick();
	// Echo("Ticks of mapping task for migration = "); Echo(itoa(now - then)); Echo("\n");

	if(task->old_proc == task->proc_idx){
		Echo("Will not migrate. Task is in the same PE as the target address\n");
		return;
	}

	Echo("Migrating task to address "); Echo(itoa(mapper->processors[task->proc_idx].addr));

	/* Allocate the page on target address */
	mapper->processors[task->proc_idx].free_page_cnt--;
	mapper->available_slots--;

	/* Mark the task as migrating */
	task->status = MIGRATING;

	/* Send migration order to Kernel at old processor address */
	Message msg;
	msg.msg[0] = TASK_MIGRATION;
	msg.msg[1] = task->id;
	msg.msg[2] = mapper->processors[task->proc_idx].addr;
	msg.length = 3;
	SSend(&msg, KERNEL_MSG | mapper->processors[task->old_proc].addr);
}

void tm_migration_complete(mapper_t *mapper, int task_id)
{
	Echo("Received migration completed to task id "); Echo(itoa(task_id)); Echo(" at time "); Echo(itoa(GetTick())); Echo("\n");

	app_t *app = app_search(mapper->apps, task_id >> 8);
	if(app == NULL){
		Echo("Not found app that migrated with id "); Echo(itoa(task_id >> 8)); Echo("\n");
		return;
	}

	task_t *task = app->task[task_id & 0xFF];
	if(task->id != task_id){
		Echo("Not found task that migrated with id "); Echo(itoa(task_id)); Echo("\n");
		return;
	}

	if(task->status != MIGRATING){
		Echo("Will not free old resources. Task not marked as migrating. Already freed?\n");
		return;
	}

	/* Mark as migrationg finished */
	task->status = RUNNING;

	/* Free old processor resources */
	mapper->processors[task->old_proc].free_page_cnt++;
	mapper->available_slots++;
}
