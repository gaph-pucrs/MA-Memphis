/**
 * MA-Memphis
 * @file mapper.c
 *
 * @author Angelo Elias Dal Zotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date March 2021
 * 
 * @brief Main mapper functions
 */

#include "mapper.h"

#include <stdlib.h>
#include <stdio.h>

#include <memphis.h>
#include <memphis/services.h>

#include "window.h"

void map_init(map_t *mapper)
{
	mapper->appid_cnt = 0;

	const size_t MAX_TASKS = memphis_get_max_tasks(&(mapper->slots));
	
	mapper->pending = NULL;

	const size_t N_PE = memphis_get_nprocs(NULL, NULL);
	mapper->pes = malloc(N_PE * sizeof(pe_t));

	if(mapper->pes == NULL){
		puts("FATAL: not enough memory");
		return;
	}

	for(int i = 0; i < N_PE; i++)
		pe_init(&(mapper->pes[i]), MAX_TASKS, map_idx_to_coord(i));

	list_init(&(mapper->apps));
}

void map_new_app(map_t *mapper, int injector, size_t task_cnt, int *descriptor, int *communication)
{
	printf("New app received at %u from %x\n", memphis_get_tick(), injector);

	app_t *app = malloc(sizeof(app_t));

	if(app == NULL){
		puts("FATAL: not enough memory");
		return;
	}

	task_t *tasks = app_init(app, mapper->appid_cnt, injector, task_cnt, descriptor, communication);
	if(tasks == NULL){
		puts("FATAL: not enough memory");
		return;
	}

	/* Search for statically mapped tasks */
	unsigned failed_cnt = 0;
	bool has_static = false;
	for(int i = 0; i < task_cnt; i++){
		int pe_addr = descriptor[i * MAP_DESCR_ENTRY_LEN];
		if(pe_addr != -1){
			int pe_idx = map_coord_to_idx(pe_addr);
			failed_cnt += task_set_pe(&tasks[i], &(mapper->pes[pe_idx]));
			has_static = true;
		}
	}

	app_set_has_static(app, has_static);

	/* Check if it will be mapped now or later */
	if(task_cnt > mapper->slots || failed_cnt > 0){
		puts("No available slots");

		app_set_failed(app, failed_cnt);
		mapper->pending = app;
		return;
	}

	/* Finally do the mapping */
	map_do(mapper, app);
}

int map_coord_to_idx(int coord)
{
	size_t PE_X_CNT;
	memphis_get_nprocs(&PE_X_CNT, NULL);
	return (coord >> 8) + (coord & 0xFF) * PE_X_CNT;
}

int map_idx_to_coord(int idx)
{
	size_t PE_X_CNT;
	memphis_get_nprocs(&PE_X_CNT, NULL);
	return ((idx % PE_X_CNT) << 8) | (idx / PE_X_CNT);
}

int map_xy_to_idx(int x, int y)
{
	size_t PE_Y_CNT;
	memphis_get_nprocs(NULL, &PE_Y_CNT);
	return x + y*PE_Y_CNT;
}

void map_do(map_t *mapper, app_t *app)
{
	size_t task_cnt;
	task_t *tasks = app_get_tasks(app, &task_cnt);

	/* Only map statically if there is any */
	unsigned static_cnt = 0;
	unsigned center_x = 0;
	unsigned center_y = 0;
	if(app_has_static(app)){
		/* 1st phase: static mapping */
		for(int i = 0; i < task_cnt; i++){
			pe_t *pe = task_get_pe(&tasks[i]);

			if(pe != NULL){
				list_entry_t *entry = pe_task_push_back(pe, &tasks[i]);

				if(entry == NULL){
					puts("FATAL: not enough memory");
					/**
					 * @todo
					 * Destroy app
					 * Destroy tasks
					 * Unnalocate pending
					 */
					return;
				}

				int addr = pe_get_addr(pe);
				center_x += addr >> 8;
				center_y += addr & 0xFF;
				static_cnt++;
			}
		}

		center_x /= static_cnt;
		center_y /= static_cnt;
	}

	/* Avoid mapping when there is no dynamically allocated task */
	if(static_cnt != task_cnt){
		/* 2nd phase: sliding window map */

		/* 1st step: select a window */
		int wx = MAP_MIN_WX;
		int wy = MAP_MIN_WY;

		/* Pre-growth due to minimum size */
		size_t PE_SLOTS = memphis_get_max_tasks(NULL);
		while(wx*wy*PE_SLOTS < task_cnt){
			if(wy <= wx){
				wy++;	/* Prefer to have y greater than x because window slides in x-axis */
			} else {
				wx++;
			}
		}

		size_t PE_X_CNT;
		size_t PE_Y_CNT;
		memphis_get_nprocs(&PE_X_CNT, &PE_Y_CNT);

		if(wx > PE_X_CNT)
			wx = PE_X_CNT;

		if(wy > PE_Y_CNT)
			wy = PE_Y_CNT;

		wdo_t window;
		if(static_cnt != 0){
			/* Set the window based on the center of the statically mapped tasks */
			wdo_init(&window, center_x - wx/2, center_y - wy/2, wx, wy);
			wdo_from_center(&window, mapper->pes, task_cnt);
		} else {
			/* Set the window based on the last one selected */
			static bool initialized = false;
			static int last_x;
			static int last_y;
			
			if(wx != MAP_MIN_WX || wy != MAP_MIN_WY || !initialized){
				last_x = PE_X_CNT - wx;
				last_y = PE_Y_CNT - wy;
				initialized = true;
			}

			wdo_init(&window, last_x, last_y, wx, wy);
			wdo_from_last(&window, mapper->pes, task_cnt);
		}

		/* 2nd step: get the mapping order */
		list_t *ordered_tasks = app_get_order(app);

		if(ordered_tasks == NULL){
			puts("FATAL: not enough memory");
			/**
			 * @todo
			 * Destroy app
			 * Destroy tasks
			 * Unnalocate pending
			 */
			return;
		}

		/* 3rd step: map with the least communication cost and most parallelism */
		list_entry_t *entry = list_front(ordered_tasks);
		while(entry != NULL){
			task_t *task = list_get_data(entry);
			pe_t *pe = task_get_pe(task);
			if(pe == NULL){	/* Skip tasks already mapped */
				pe_t *pe = task_map(task, mapper->pes, &window);

				task_set_pe(task, pe);
				pe_task_push_back(pe, task);
			}

			entry = list_next(entry);
		}

		list_destroy(ordered_tasks);
		free(ordered_tasks);
	}

	/* Send allocation message */
	size_t out_size = (task_cnt * 2 + 1) << 2;
	int *out_msg = malloc(out_size);
	out_msg[0] = APP_ALLOCATION_REQUEST;
	int *payload = &(out_msg[1]);
	for(int i = 0; i < task_cnt; i++){
		payload[i*2] = task_get_id(&tasks[i]);
		pe_t *pe = task_get_pe(&tasks[i]);
		payload[i*2 + 1] = pe_get_addr(pe);
	}

	memphis_send_any(out_msg, out_size, app_get_injector(app));

	free(out_msg);
	out_msg = NULL;

	/* Now compute the mapping score */
	unsigned edges = 0;
	unsigned cost = 0;
	for(int i = 0; i < task_cnt; i++){
		task_t *vertex = &(tasks[i]);
		pe_t *vertex_pe = task_get_pe(vertex);

		list_t *succs = task_get_succs(vertex);
		list_entry_t *succ = list_front(succs);
		while(succ != NULL){
			task_t *succ_task = list_get_data(succ);
			pe_t *succ_pe = task_get_pe(succ_task);
			cost += map_manhattan(pe_get_addr(vertex_pe), pe_get_addr(succ_pe));
			edges++;

			succ = list_next(succ);
		}
	}

	/* Single precision is enough */
	float score = (edges > 0) ? ((float)cost / (float)edges) : 0; /* Careful with division by zero */
	
	/* Printf with 2 decimal places and avoid linking to float printf */
	printf("Mapped with score %u.%u at %u\n", (int)score, (int)(score*100 - (int)score*100), memphis_get_tick());

	app_set_score(app, score);

	/* Management app has special requirements */
	if(mapper->appid_cnt == 0){
		/* Mapper task is already allocated */
		unsigned allocated = app_allocated(app);

		/* Check if mapper task is the only MA task */
		if(allocated == task_cnt){
			/* Then finish mapping */
			app_mapping_complete(app);
			mapper->appid_cnt++;
		}
	}

	mapper->slots -= task_cnt;
	list_push_back(&(mapper->apps), app);
}

unsigned map_manhattan(int a, int b)
{
	int a_x = a >> 8;
	int a_y = a & 0xFF;
	int b_x = b >> 8;
	int b_y = b & 0xFF;

	int dist_x = abs(a_x - b_x);
	int dist_y = abs(a_y - b_y);

	return dist_x + dist_y;
}

void map_task_allocated(map_t *mapper, int id)
{
	printf("Received task allocated from id %d\n", id);

	int appid = id >> 8;
	list_entry_t *entry = list_find(&(mapper->apps), &appid, app_find_fnc);

	if(entry == NULL){
		puts("App not found");
		return;
	}

	app_t *app = list_get_data(entry);

	size_t alloc_cnt = app_add_allocated(app);
	size_t task_cnt;
	task_t *tasks = app_get_tasks(app, &task_cnt);
	if(alloc_cnt != task_cnt)
		return;

	/* All tasks allocated, send task release */
	printf("Sending TASK_RELEASE at time %d for app %d\n", memphis_get_tick(), appid);

	/* Assemble and send task release */
	const size_t msg_size = (task_cnt + 3) << 2;
	int *out_msg = malloc(msg_size);

	out_msg[0] = TASK_RELEASE;
	// out_msg[1] = appid_shift | i;
	out_msg[2] = task_cnt;

	for(int i = 0; i < task_cnt; i++){
		task_t *task = &(tasks[i]);

		/* Mark task as running */
		task_release(task);

		pe_t *pe = task_get_pe(task);
		int addr = pe_get_addr(pe);
		out_msg[i + 3] = addr;
	}

	int appid_shift = appid << 8;
	for(int i = 0; i < task_cnt; i++){
		/* Tell kernel to populate the proper task by sending the ID */
		out_msg[1] = appid_shift | i;

		/* Send message directed to kernel at task address */
		memphis_send_any(out_msg, msg_size, MEMPHIS_KERNEL_MSG | out_msg[i + 3]);
	}

	free(out_msg);
	out_msg = NULL;

	app_mapping_complete(app);
	mapper->appid_cnt++;
}

void map_task_terminated(map_t *mapper, int id)
{
	printf("Received task terminated from id %d at time %d\n", id, memphis_get_tick());

	int appid = id >> 8;
	list_entry_t *entry = list_find(&(mapper->apps), &appid, app_find_fnc);

	if(entry == NULL){
		puts("App not found");
		return;
	}

	app_t *app = list_get_data(entry);

	size_t task_cnt;
	task_t *tasks = app_get_tasks(app, &task_cnt);

	const int taskid = id & 0xFF;
	task_t *task = &(tasks[taskid]);

	pe_t *pe = task_get_pe(task);
	mapper->slots++;
	if(pe_task_remove(pe, task) && mapper->pending != NULL)
		app_rem_failed(mapper->pending);

	pe_t *old_pe = task_destroy(task);
	if(old_pe != NULL){
		/* The task finished with a migration request on the fly */
		mapper->slots++;
		if(pe_task_remove(old_pe, task) && mapper->pending != NULL)
			app_rem_failed(mapper->pending);
	}

	size_t alloc_cnt = app_rem_allocated(app);

	/* All tasks terminated, terminate app */
	if(alloc_cnt == 0){
		printf("App %d terminated at time %d\n", app->id, memphis_get_tick());

		app_destroy(app);
		task = NULL;
		tasks = NULL;
		list_remove(&(mapper->apps), entry);

		free(app);
		app = NULL;
	}

	if(mapper->pending == NULL)
		return;

	/* There is a pending application which could not be allocated before */
	app = mapper->pending;
	tasks = app_get_tasks(app, &task_cnt);
	if(mapper->slots < task_cnt)
		return;

	/* Able to map unless it was due to static mapping not available */
	unsigned failed_cnt = app_get_failed(app);
	if(failed_cnt != 0)
		return;

	/* Application is able to map (enough slots) and static mapping is OK */
	map_do(mapper, app);
	mapper->pending = NULL;
}
