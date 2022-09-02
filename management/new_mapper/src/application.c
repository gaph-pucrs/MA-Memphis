/**
 * MA-Memphis
 * @file application.c
 *
 * @author Angelo Elias Dal Zotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date March 2021
 * 
 * @brief Application structures for the mapper
 */

#include "application.h"

#include <stdlib.h>

#include <memphis.h>
#include <memphis/services.h>

#include "mapper.h"

task_t *app_init(app_t *app, int id, int injector, size_t task_cnt, int *descriptor, int *communication)
{
	app->id = id;
	app->injector = injector;
	app->task_cnt = task_cnt;
	app->allocated_cnt = 0;
	app->failed_cnt = 0;
	app->has_static = false;
	app->score = 0;

	app->tasks = malloc(task_cnt * sizeof(task_t));

	if(app->tasks == NULL)
		return NULL;

	for(int i = 0; i < task_cnt; i++){		
		int tag = descriptor[i * MAP_DESCR_ENTRY_LEN + 1];
		task_init(&(app->tasks[i]), id, i, tag);
	}

	int comm_idx = 0;
	for(int i = 0; i < task_cnt; i++){
		int encoded_succ;
		do {
			encoded_succ = communication[comm_idx++];
			int succ_id = abs(encoded_succ) - 1;

			if(succ_id >= 0){
				int res = task_comm_push_back(&(app->tasks[i]), &(app->tasks[succ_id]));
				
				if(res != 0){
					free(app->tasks);
					return NULL;
				}
			}
			
		} while(encoded_succ > 0);
	}

	return app->tasks;
}

void app_set_failed(app_t *app, unsigned failed_cnt)
{
	app->failed_cnt = failed_cnt;
}

void app_set_has_static(app_t *app, bool has_static)
{
	app->has_static = has_static;
}

bool app_has_static(app_t *app)
{
	return app->has_static;
}

task_t *app_get_tasks(app_t *app, size_t *task_cnt)
{
	*task_cnt = app->task_cnt;
	return app->tasks;
}

list_t *app_get_order(app_t *app)
{
	const size_t TASK_CNT = app->task_cnt;

	list_t initials;
	list_init(&initials);

	/* Make an initial tasks list */
	for(int i = 0; i < TASK_CNT; i++){
		task_t *task = &(app->tasks[i]);
		list_t *preds = task_get_preds(task);
		if(list_empty(preds)){
			list_entry_t *entry = list_push_back(&initials, task);
			if(entry == NULL){
				/**
				 * @todo
				 * Clear remaining entries
				 */
				return NULL;				
			}
		}
	}

	list_t *order = malloc(sizeof(list_t));
	list_init(order);

	if(order == NULL)
		return NULL;

	/* The mapping order starts in each initial task */
	list_entry_t *entry = list_front(&initials);
	while(entry != NULL){
		task_t *task = list_get_data(entry);
		list_entry_t *pushed = list_push_back(order, task);
		if(pushed == NULL){
			/**
			 * @todo
			 * Clear remaining entries
			 */
			return NULL;
		}

		order = task_get_order(task, order);
		if(order == NULL)
			return NULL;

		entry = list_next(entry);
	}

	/* Solve one or more cyclic dependencies */
	if(list_get_size(order) == TASK_CNT)
		return order;
	
	/* Solve one or more cyclic dependencies */
	for(int i = 0; i < TASK_CNT; i++){
		task_t *task = &(app->tasks[i]);
		if(list_find(order, task, NULL) != NULL)
			continue;

		list_entry_t *pushed = list_push_back(order, task);
		if(pushed == NULL){
			/**
			 * @todo
			 * Clear remaining entries
			 */
			return NULL;
		}

		order = task_get_order(task, order);
		if(order == NULL)
			return NULL;
	}

	return order;
}

void app_set_score(app_t *app, float score)
{
	app->score = score;
}

unsigned app_allocated(app_t *app)
{
	app->allocated_cnt++;
	return app->allocated_cnt;
}

void app_mapping_complete(app_t *app)
{
	int out_msg = APP_MAPPING_COMPLETE;
	memphis_send_any(&out_msg, 4, app->injector);

	if(app->id == 0)
		memphis_br_send_all(0, RELEASE_PERIPHERAL);
}

int app_get_injector(app_t *app)
{
	return app->injector;
}

bool app_find_fnc(void *data, void *cmpval)
{
	app_t *app = (app_t*)data;
	int appid = *((int*)cmpval);

	return (app->id == appid);
}

size_t app_add_allocated(app_t *app)
{
	app->allocated_cnt++;
	return app->allocated_cnt;
}

size_t app_rem_allocated(app_t *app)
{
	app->allocated_cnt--;
	return app->allocated_cnt;
}

void app_destroy(app_t *app)
{
	free(app->tasks);
}

unsigned app_get_failed(app_t *app)
{
	return app->failed_cnt;
}

void app_rem_failed(app_t *app)
{
	app->failed_cnt--;
}

int app_get_id(app_t *app)
{
	return app->id;
}

task_t *app_get_task(app_t *app, int taskid)
{
	if(taskid >= app->task_cnt)
		return NULL;

	return &(app->tasks[taskid]);
}
