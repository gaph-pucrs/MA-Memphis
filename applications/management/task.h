#pragma once

typedef struct _task {
	int id;
	int proc_idx;
	unsigned code_sz;
	unsigned data_sz;
	unsigned bss_sz;
	unsigned init_addr;
} task_t;

task_t *task_get_free(task_t *tasks);
void task_init(task_t *tasks);


