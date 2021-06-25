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

	task_t *consumers[PKG_MAX_TASKS_APP - 1];
};

task_t *task_get_free(task_t *tasks);
void task_init(task_t *tasks);
void task_order_consumers(task_t *order[], unsigned *ordered, unsigned *order_idx, int task_cnt);
bool task_is_ordered(task_t *task, task_t *order[], unsigned order_cnt);
unsigned task_get_predecessors(task_t *task, app_t *app, task_t *predecessors[]);
int task_terminate(task_t *task, unsigned max_consumers);
