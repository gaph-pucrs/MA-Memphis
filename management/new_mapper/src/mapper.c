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

bool _initialized = false;
size_t _PE_CNT;
size_t _PE_X_CNT;
size_t _PE_Y_CNT;
size_t _PE_SLOTS;
size_t _MC_SLOTS;

void map_init(map_t *mapper)
{
	mapper->appid_cnt = 0;
	mapper->pending = NULL;

	if(!_initialized){
		puts("TODO: FATAL");
		while(true);
	}

	mapper->pes = malloc(_pe_cnt * sizeof(pe_t));

	if(mapper->pes == NULL){
		puts("FATAL: not enough memory");
		return;
	}

	for(int i = 0; i < _pe_cnt; i++)
		pe_init(&(mapper->pes[i]), map_idx_to_coord(i));
}

void map_new_app(map_t *mapper, size_t task_cnt, int *descriptor, int *communication)
{
	printf("New app received at %d\n", memphis_get_tick());

	app_t *app = malloc(sizeof(app_t));

	if(app == NULL){
		puts("FATAL: not enough memory");
		return;
	}

	task_t *tasks = app_init(app, mapper->appid_cnt, task_cnt, descriptor, communication);
	if(tasks == NULL){
		puts("FATAL: not enough memory");
		return;
	}

	/* Search for statically mapped tasks */
	int failed_cnt = 0;
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
	return (coord >> 8) + (coord & 0xFF) * _PE_X_CNT;
}

int map_idx_to_coord(int idx)
{
	return ((idx % _PE_X_CNT) << 8) | (idx / _PE_X_CNT);
}

int map_xy_to_id(int x, int y)
{
	return x + y*_PE_Y_CNT;
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
		while(wx*wy*_PE_SLOTS < task_cnt){
			if(wy <= wx){
				wy++;	/* Prefer to have y greater than x because window slides in x-axis */
			} else {
				wx++;
			}
		}

		if(wx > _PE_X_CNT)
			wx = _PE_X_CNT;

		if(wy > _PE_Y_CNT)
			wy = _PE_Y_CNT;

		wdo_t window;
		if(static_cnt != 0){
			/* Set the window based on the center of the statically mapped tasks */
			wdo_init(&window, center_x - wx/2, center_y - wy/2, wx, wy);
			wdo_from_center(&window, mapper->pes, task_cnt);
		} else {
			/* Set the window based on the last one selected */
			static int last_x = _PE_X_CNT - MAP_MIN_WX;
			static int last_y = _PE_Y_CNT - MAP_MIN_WY;

			if(wx != MAP_MIN_WX || wy != MAP_MIN_WY){
				last_x = _PE_X_CNT - wx;
				last_y = _PE_Y_CNT - wy;
			}

			wdo_init(&window, last_x, last_y, wx, wy);
			wdo_slide(&window);

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
				pe_t *pe = task_map(task, pes, window);

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

	/**
	 * @todo
	 * How to identify injectors -> Embed address in NEW_APP message
	 * and save in app_t
	 */
	memphis_send_any(out_msg, out_size, @todo);

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
			pe_t *succ_pe = task_get_pe(succ);
			cost += map_manhattan(pe_get_addr(vertex_pe), pe_get_addr(succ_pe));
			edges++;

			succ = list_next(succ);
		}
	}

	/* Single precision is enough */
	float score = (edges > 0) ? ((float)cost / (float)edges) : 0; /* Careful with division by zero */
	
	/* Printf with 2 decimal places and avoid linking to float printf */
	printf("Mapped with score %u.%u at %u", (int)score, (int)(score*100), memphis_get_tick());

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
