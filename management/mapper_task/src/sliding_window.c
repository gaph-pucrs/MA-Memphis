#include <stddef.h>

#include "sliding_window.h"
#include "mapper.h"

int sw_map_task(app_t *app, task_t *task, processor_t *processors, window_t *window)
{
	unsigned cost = -1; /* Start at infinite cost */
	int sel_x = -1;
	int sel_y = -1;

	task_t *predecessors[PKG_MAX_TASKS_APP - 1];
	unsigned pred_cnt = task_get_predecessors(task, app, predecessors);

	for(int x = window->x; x < window->x + window->wx; x++){
		for(int y = window->y; y < window->y + window->wy; y++){	/* Traverse Y first */
			processor_t *pe = processors_get(processors, x, y);
			
			if(pe->free_page_cnt == 0)	/* Skip full PEs */
				continue;

			unsigned c = 0;

			/* 1st: Keep tasks from different apps apart from each other */
			c += (PKG_MAX_LOCAL_TASKS - (pe->free_page_cnt + pe->pending_map_cnt)) * 4;

			/* 2nd: Keep tasks from the same app apart */
			c += pe->pending_map_cnt * 2;

			/* 3rd: Add a cost for each hop in consumer tasks */
			for(int t = 0; t < app->task_cnt - 1 && task->consumers[t] != NULL; t++){
				task_t *consumer = task->consumers[t];
				if(consumer->proc_idx != -1)	/* Manhattan distance from mapped consumers */
					c += map_manhattan_distance(x << 8 | y, processors[consumer->proc_idx].addr);
			}

			/* 4th: Add a cost for each hop in producer tasks */
			for(int t = 0; t < pred_cnt; t++){
				task_t *producer = predecessors[t];
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
