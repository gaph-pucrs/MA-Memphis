/**
 * MA-Memphis
 * @file sliding_window.c
 *
 * @author Angelo Elias Dalzotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date June 2021
 * 
 * @brief Sliding window mapping heuristic
 */

#include <stddef.h>

#include "sliding_window.h"
#include "mapper.h"

void sw_map_app(app_t *app, processor_t *processors)
{
	/* 1st step: select a window */
	window_t window;
	window_search(&window, processors, app);
	// printf("Selected window %dx%d\n", window.x, window.y);

	/* 2nd step: get the mapping order */
	task_t *mapping_order[app->task_cnt];
	app_get_order(app, mapping_order);

	/* 3rd step: map with the least communication cost and most parallelism */
	sw_map_dynamic(app, mapping_order, processors, &window);
}

void sw_map_dynamic(app_t *app, task_t *order[], processor_t *processors, window_t *window)
{
	for(int i = 0; i < app->task_cnt; i++){
		task_t *task = order[i];

		if(task->proc_idx != -1)	/* Skip tasks already mapped */
			continue;

		int seq = sw_map_task(task, app, processors, window);

		task->proc_idx = seq;
		processors[seq].free_page_cnt--;
		processors[seq].pending_map_cnt++;
		// printf("Dinamically mapped task %d at address %x\n", task->id, processors[seq].addr);
	}
}

int sw_map_task(task_t *task, app_t *app, processor_t *processors, window_t *window)
{
	unsigned cost = -1; /* Start at infinite cost */
	int sel_x = -1;
	int sel_y = -1;

	task_t *producers[PKG_MAX_TASKS_APP - 1];
	unsigned pred_cnt = task_get_producers(task, app, producers);

	for(int x = window->x; x < window->x + window->wx; x++){
		for(int y = window->y; y < window->y + window->wy; y++){	/* Traverse Y first */
			processor_t *pe = processors_get(processors, x, y);
			
			if(pe->free_page_cnt == 0)	/* Skip full PEs */
				continue;

			unsigned c = 0;

			/* 1st: Keep tasks from different apps apart from each other */
			c += (PKG_MAX_LOCAL_TASKS - (pe->free_page_cnt + pe->pending_map_cnt)) << 4;

			/* 2nd: Keep tasks from the same app apart */
			c += pe->pending_map_cnt << 3;

			/* 3rd: Add a cost for each hop in consumer tasks */
			for(int t = 0; t < app->task_cnt - 1 && task->consumers[t] != NULL; t++){
				task_t *consumer = task->consumers[t];
				if(consumer->proc_idx != -1)	/* Manhattan distance from mapped consumers */
					c += map_manhattan_distance(x << 8 | y, processors[consumer->proc_idx].addr);
			}

			/* 4th: Add a cost for each hop in producer tasks */
			for(int t = 0; t < pred_cnt; t++){
				task_t *producer = producers[t];
				if(producer->proc_idx != -1)
					c += map_manhattan_distance(x << 8 | y, processors[producer->proc_idx].addr);
			}

			if(c == 0){
				return x + y*PKG_N_PE_X;
			} else if(c < cost){
				cost = c;
				sel_x = x;
				sel_y = y;
			}
		}
	}
	return sel_x + sel_y*PKG_N_PE_X;
}
