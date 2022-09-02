#include "task.h"

#include <memphis.h>

#include "mapper.h"
#include "window.h"

void task_init(task_t *task, int appid, int taskid, unsigned tag)
{
	task->id = (appid << 8) | taskid;
	task->tag = tag;

	task->pe = NULL;
	task->old_pe = NULL;
	
	list_init(&(task->preds));
	list_init(&(task->succs));

	task->status = TASK_STATUS_BLOCKED;
}

int task_comm_push_back(task_t *vertex, task_t *succ)
{
	list_entry_t *succ_entry = list_push_back(&(vertex->succs), succ);
	list_entry_t *pred_entry = list_push_back(&(succ->preds), vertex);

	return (succ_entry == NULL || pred_entry == NULL);
}

bool task_set_pe(task_t *task, pe_t *pe)
{
	task->pe = pe;
	return pe_add_pending(pe);
}

pe_t *task_get_pe(task_t *task)
{
	return task->pe;
}

pe_t *task_get_old_pe(task_t *task)
{
	return task->old_pe;
}

list_t *task_get_succs(task_t *task)
{
	return &(task->succs);
}

list_t *task_get_preds(task_t *task)
{
	return &(task->preds);
}

list_t *task_get_order(task_t *task, list_t *order)
{
	list_entry_t *succ = list_front(&(task->succs));
	while(succ != NULL){
		task_t *succ_task = list_get_data(succ);
		if(list_find(order, succ_task, NULL) == NULL){
			list_entry_t *pushed = list_push_back(order, succ_task);
			if(pushed == NULL){
				/**
				 * @todo
				 * Clear remainings
				 */
				return NULL;
			}
		}

		succ = list_next(succ);
	}

	return order;
}

pe_t *task_map(task_t *task, pe_t *pes, wdo_t *window)
{
	unsigned cost = -1; 	/* Start at infinite cost */
	pe_t *sel = NULL;
	pe_t *old = task->pe;	/* Current mapped PE */
	
	size_t PE_SLOTS = memphis_get_max_tasks(NULL);
	for(int x = window->x; x < window->x + window->wx; x++){
		for(int y = window->y; y < window->y + window->wy; y++){	/* Traverse Y first */
			pe_t *pe = &(pes[map_xy_to_idx(x, y)]);
			
			int is_old_pe = (pe == old);
			int free_slots = pe_get_slots(pe) + is_old_pe;	/* Add a free page in the current mapped */

			if(free_slots == 0)	/* Skip full PEs */
				continue;

			const int appid = task->id >> 8;
			int same_app_allocated = 0;
			list_t *mapped = pe_get_mapped(pe);
			list_entry_t *entry = list_front(mapped);
			while(entry != NULL){
				task_t *mapped = list_get_data(entry);
				if(mapped && mapped->id >> 8 == appid)
					same_app_allocated++;

				entry = list_next(entry);
			}
			same_app_allocated -= is_old_pe;

			unsigned c = 0;
			/* 1st: Keep tasks from different apps apart from each other */
			c += (PE_SLOTS - (free_slots + same_app_allocated)) << 2;

			/* 2nd: Keep tasks from the same app apart */
			c += same_app_allocated << 1;

			/* 3rd: Add a cost for each hop in successor tasks */
			entry = list_front(&(task->succs));
			while(entry != NULL){
				task_t *succ = list_get_data(entry);
				if(succ->pe != NULL)
					c += map_manhattan(pe_get_addr(pe), pe_get_addr(succ->pe));

				entry = list_next(entry);
			}

			/* 4th: Add a cost for each hop in predecessor tasks */
			entry = list_front(&(task->preds));
			while(entry != NULL){
				task_t *pred = list_get_data(entry);
				if(pred->pe != NULL)
					c += map_manhattan(pe_get_addr(pe), pe_get_addr(pred->pe));

				entry = list_next(entry);
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

int task_get_id(task_t *task)
{
	return task->id;
}

void task_release(task_t *task)
{
	task->status = TASK_STATUS_RUNNING;
}

pe_t *task_destroy(task_t *task)
{
	list_destroy(&(task->preds));
	list_destroy(&(task->succs));

	task->status = TASK_STATUS_TERMINATED;

	return task->old_pe;
}

bool task_is_allocated(task_t *task)
{
	return (task->status != TASK_STATUS_TERMINATED);
}

bool task_is_migrating(task_t *task)
{
	return (task->status == TASK_STATUS_MIGRATING);
}

unsigned task_get_tag(task_t *task)
{
	return task->tag;
}

list_entry_t *task_migrate(task_t *task, pe_t *pe)
{
	task->old_pe = task->pe;
	task->pe = pe;
	task->status = TASK_STATUS_MIGRATING;
	pe_add_pending(pe);
	return pe_task_push_back(pe, task);
}
