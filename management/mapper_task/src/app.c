#include <stddef.h>

#include "app.h"

app_t *app_get_free(app_t *apps)
{
	for(int i = 0; i < PKG_MAX_LOCAL_TASKS*PKG_N_PE; i++)
		if(apps[i].id == -1)
			return &apps[i];

	return NULL;
}

void app_init(app_t *apps)
{
	for(int i = 0; i < PKG_MAX_LOCAL_TASKS*PKG_N_PE; i++){
		apps[i].id = -1;
		for(int j = 0; j < PKG_MAX_TASKS_APP; j++)
			apps[i].task[j] = NULL;
	}
}

app_t *app_search(app_t *apps, int appid)
{
	for(int i = 0; i < PKG_MAX_TASKS_APP; i++){
		if(apps[i].id == appid)
			return &apps[i];
	}
	return NULL;
}
