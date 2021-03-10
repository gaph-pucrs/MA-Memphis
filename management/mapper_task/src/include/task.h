#pragma once

typedef enum _task_status {
	BLOCKED,
	RUNNING,
	MIGRATING
} task_status_t;

typedef struct _task {
	int id;
	int proc_idx;
	int old_proc;
	unsigned code_sz;
	unsigned data_sz;
	unsigned bss_sz;
	unsigned init_addr;
	task_status_t status;
} task_t;

task_t *task_get_free(task_t *tasks);
void task_init(task_t *tasks);
