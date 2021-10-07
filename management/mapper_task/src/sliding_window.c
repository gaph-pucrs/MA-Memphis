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
#include <stdio.h>

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

	// for(int i = 0; i < app->task_cnt; i++)
	// 	printf("Order %d = %d\n", i, mapping_order[i]->id);

	/* 3rd step: map with the least communication cost and most parallelism */
	sw_map_dynamic(app, mapping_order, processors, &window);
}

void sw_map_dynamic(app_t *app, task_t *order[], processor_t *processors, window_t *window)
{
	for(int i = 0; i < app->task_cnt; i++){
		task_t *task = order[i];

		if(task->processor)	/* Skip tasks already mapped */
			continue;

		processor_t *processor = sw_map_task(task, app, processors, window);

		task->processor = processor;
		processor->free_page_cnt--;
		processor->pending_map_cnt++;
		// printf("Dinamically mapped task %d at address %x\n", task->id, processors[seq].addr);
	}
}

processor_t *sw_map_task(task_t *task, app_t *app, processor_t *processors, window_t *window)
{
	unsigned cost = -1; /* Start at infinite cost */
	processor_t *sel = NULL;

	for(int x = window->x; x < window->x + window->wx; x++){
		for(int y = window->y; y < window->y + window->wy; y++){	/* Traverse Y first */
			processor_t *pe = processors_get(processors, x, y);
			
			if(pe->free_page_cnt == 0)	/* Skip full PEs */
				continue;

			unsigned c = 0;

			/* 1st: Keep tasks from different apps apart from each other */
			c += (PKG_MAX_LOCAL_TASKS - (pe->free_page_cnt + pe->pending_map_cnt)) << 2;

			/* 2nd: Keep tasks from the same app apart */
			c += pe->pending_map_cnt << 1;

			/* 3rd: Add a cost for each hop in successor tasks */
			for(int t = 0; t < task->succ_cnt; t++){
				task_t *successor = task->successors[t];
				if(successor->processor)	/* Manhattan distance from mapped successors */
					c += map_manhattan_distance(x << 8 | y, successor->processor->addr);
			}

			/* 4th: Add a cost for each hop in predecessor tasks */
			for(int t = 0; t < task->pred_cnt; t++){
				task_t *predecessor = task->predecessors[t];
				if(predecessor->processor)
					c += map_manhattan_distance(x << 8 | y, predecessor->processor->addr);
			}

			if(c == 0){
				return pe;
			} else if(c < cost){
				cost = c;
				sel = pe;
			}
		}
	}
	return sel;
}
