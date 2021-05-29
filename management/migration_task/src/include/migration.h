#pragma once

#include <stdbool.h>

typedef struct _migration_task {
	int id;
	int missed_cnt;
} migration_task_t;

typedef struct _migration_ring {
	migration_task_t tasks[PKG_PENDING_SVC_MAX];
	unsigned char head;
	// unsigned char tail;
	// bool empty;
	// bool full;
} migration_ring_t;

void migration_init();
void migration_test(int id, int remaining);
migration_task_t *migration_search_task(int id);
migration_task_t *migration_task_insert(int id);
void migration_update_deadline_miss(migration_task_t *task);
int migration_miss_count(migration_task_t *task);
void migration_clear_miss(migration_task_t *task);
