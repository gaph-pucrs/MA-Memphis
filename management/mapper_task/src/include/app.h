#pragma once

#include "task.h"

/* Forward declaration */
typedef struct _task task_t;

typedef struct _app {
	int id;
	unsigned task_cnt;
	unsigned allocated_cnt;
	unsigned mapping_score;
	task_t *task[PKG_MAX_TASKS_APP];
	bool has_static_tasks;
	int center_x;
	int center_y;
} app_t;

void app_init(app_t *apps);
app_t *app_get_free(app_t *apps);
void app_build(app_t *app, int id, unsigned task_cnt, int *descriptor, int *communication, task_t *tasks);
app_t *app_search(app_t *apps, int appid);
void app_get_order(app_t *app, task_t *order[]);
