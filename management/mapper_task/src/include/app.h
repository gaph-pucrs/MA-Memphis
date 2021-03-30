#pragma once

#include "task.h"
#include "ma_pkg.h"

typedef struct _app {
	int id;
	unsigned task_cnt;
	unsigned allocated_cnt;
	task_t *task[PKG_MAX_TASKS_APP];
} app_t;

app_t *app_get_free(app_t *apps);
void app_init(app_t *apps);
app_t *app_search(app_t *apps, int appid);
