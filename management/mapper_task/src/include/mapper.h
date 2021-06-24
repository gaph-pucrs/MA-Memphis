#pragma once

#include <stdbool.h>

#include "app.h"
#include "processor.h"

#define TASK_DESCRIPTOR_SIZE 2

#define MAP_STRIDE 2
#define MAP_MIN_WX 3
#define MAP_MIN_WY 3

typedef struct _mapper {
	unsigned pending_task_cnt;

	int pending_descr[PKG_MAX_TASKS_APP * 2];
	int pending_comm[PKG_MAX_TASKS_APP*(PKG_MAX_TASKS_APP - 1)];
	app_t *pending_map_app;

	unsigned fail_map_cnt;

	int available_slots;
	int appid_cnt;
	app_t apps[PKG_MAX_LOCAL_TASKS*PKG_N_PE];
	task_t tasks[PKG_MAX_LOCAL_TASKS*PKG_N_PE];
	processor_t processors[PKG_N_PE];
} mapper_t;

typedef struct _window {
	int x;
	int y;

	int wx;
	int wy;
} window_t;

void map_init(mapper_t *mapper);
void map_new_app(mapper_t *mapper, unsigned task_cnt, int *descriptor, int *communication);
app_t *map_build_app(mapper_t *mapper, int appid, unsigned task_cnt, int *descriptor, int *communication);
unsigned map_try_static(app_t *app, processor_t *processors);
void map_static_tasks(app_t *app, processor_t *processors);
void map_sliding_window(app_t *app, processor_t *processors);
void map_select_window(app_t *app, processor_t *processors, window_t *window);
void map_next_window(window_t *window);
unsigned map_window_pages(processor_t *processors, window_t *window);
void map_get_order(app_t *app, task_t *order[]);
void map_order_consumers(task_t *order[], unsigned *ordered, unsigned *order_idx, int task_cnt);
bool map_is_task_ordered(task_t *order[], task_t *task, unsigned order_cnt);
void map_dynamic_tasks(app_t *app, task_t *order[], processor_t *processors, window_t *window);
void map_set_score(app_t *app, processor_t *processors);
void map_task_allocated(mapper_t *mapper, int id);
void map_task_terminated(mapper_t *mapper, int id);
void map_task_allocation(app_t *app, processor_t *processors);
void map_try_mapping(mapper_t *mapper, int appid, int task_cnt, int *descr, int *comm, processor_t *processors);
void map_task_release(mapper_t *mapper, app_t *app);
void map_app_mapping_complete(app_t *app);
task_t *map_nearest_tag(mapper_t *mapper, app_t *ma, int address, unsigned tag);
unsigned map_manhattan_distance(int source, int target);
void map_request_service(mapper_t *mapper, int address, unsigned tag, int requester);
