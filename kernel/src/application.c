/**
 * MA-Memphis
 * @file application.c
 *
 * @author Angelo Elias Dal Zotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date August 2022
 *
 * @brief Control location of communicating tasks
 */

#include "application.h"

#include <stdlib.h>
#include <string.h>

#include <mutils/list.h>

list_t _apps;

void app_init()
{
	list_init(&_apps);
}

bool _app_find_fnc(void *data, void *cmpval)
{
	app_t *app = (app_t*)data;
	int appid = *((int*)cmpval);

	return (app->id == appid);
}

app_t *app_find(int appid)
{
	list_entry_t *entry = list_find(&_apps, &appid, _app_find_fnc);

	if(entry == NULL)
		return NULL;

	return list_get_data(entry);
}

app_t *app_emplace_back(int appid)
{
	app_t *app = malloc(sizeof(app_t));
	
	if(app == NULL)
		return NULL;

	app->id = appid;
	app->ref_cnt = 0;
	app->task_cnt = 0;
	app->task_location = NULL;

	list_push_back(&_apps, app);

	return app;
}

void app_refer(app_t *app)
{
	app->ref_cnt++;
}

void _app_remove(app_t *app)
{
	list_entry_t *entry = list_find(&_apps, app, NULL);
	
	if(entry == NULL)
		return;

	list_remove(&_apps, entry);
	free(app);
}

void app_derefer(app_t *app)
{
	app->ref_cnt--;

	if(app->ref_cnt != 0)
		return;
	
	_app_remove(app);
}

int app_get_address(app_t *app, int task)
{
	if(task >= app->task_cnt || app->task_location == NULL)
		return -1;

	return app->task_location[task];
}

void app_update(app_t *app, int task, int addr)
{
	if(app->task_location != NULL)
		app->task_location[task & 0x00FF] = addr;
}

void app_set_location(app_t *app, size_t task_cnt, int *task_location)
{
	free(app->task_location);

	app->task_location = task_location;
	app->task_cnt = task_cnt;
}

int app_copy_location(app_t *app, size_t task_cnt, int *task_location)
{
	if(task_cnt != app->task_cnt){
		free(app->task_location);
		app->task_location = NULL;
		app->task_cnt = task_cnt;
	}

	if(app->task_location == NULL)
		app->task_location = malloc(task_cnt*sizeof(int));

	if(app->task_location == NULL)
		return -1;

	memcpy(app->task_location, task_location, task_cnt*sizeof(int));

	return task_cnt;
}

size_t app_get_task_cnt(app_t *app)
{
	return app->task_cnt;
}

int *app_get_locations(app_t *app)
{
	return app->task_location;
}
