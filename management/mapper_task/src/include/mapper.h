#pragma once

#include <stdbool.h>

#include <api.h>
#include "app.h"
#include "processor.h"

#define TASK_DESCRIPTOR_SIZE 7

typedef struct _mapper {
	unsigned pending_task_cnt;

	unsigned pending_descr_sz;
	int pending_descr[MSG_SIZE - 3];
	app_t *pending_map_app;

	unsigned fail_map_cnt;

	int available_slots;
	int appid_cnt;
	app_t apps[PKG_MAX_LOCAL_TASKS*PKG_N_PE];
	task_t tasks[PKG_MAX_LOCAL_TASKS*PKG_N_PE];
	processor_t processors[PKG_N_PE];
} mapper_t;

void map_init(mapper_t *mapper);
void map_new_app(mapper_t *mapper, unsigned task_cnt, int *descriptor, unsigned desc_sz);
app_t *map_build_app(mapper_t *mapper, int appid, int *descriptor, unsigned task_cnt);
bool map_app_mapping(app_t *app, processor_t *processors);
void map_task_allocated(mapper_t *mapper, int id);
void map_task_terminated(mapper_t *mapper, int id);
void map_task_allocation(app_t *app, processor_t *processors);
void map_try_mapping(mapper_t *mapper, int appid, int *descr, int task_cnt, processor_t *processors);
void map_task_release(mapper_t *mapper, app_t *app);
void map_app_mapping_complete(app_t *app);
