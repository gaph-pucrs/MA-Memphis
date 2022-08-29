/**
 * MA-Memphis
 * @file application.h
 *
 * @author Angelo Elias Dal Zotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date March 2021
 * 
 * @brief Application structures for the mapper
 */

#pragma once

#include "task.h"

typedef struct _app {
	int id;
	int injector;
	size_t task_cnt;
	size_t allocated_cnt;

	unsigned failed_cnt;
	
	task_t *tasks;

	float score;
	bool has_static;
} app_t;

task_t *app_init(app_t *app, int id, int injector, size_t task_cnt, int *descriptor, int *communication);

void app_set_failed(app_t *app, unsigned failed_cnt);

void app_set_has_static(app_t *app, bool has_static);

bool app_has_static(app_t *app);

task_t *app_get_tasks(app_t *app, size_t *task_cnt);

list_t *app_get_order(app_t *app);

void app_set_score(app_t *app, float score);

unsigned app_allocated(app_t *app);

void app_mapping_complete(app_t *app);

int app_get_injector(app_t *app);

bool app_find_fnc(void *data, void *cmpval);

size_t app_add_allocated(app_t *app);

size_t app_rem_allocated(app_t *app);

void app_destroy(app_t *app);

unsigned app_get_failed(app_t *app);

void app_rem_failed(app_t *app);
