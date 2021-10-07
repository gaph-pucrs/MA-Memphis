/**
 * MA-Memphis
 * @file app.h
 *
 * @author Angelo Elias Dalzotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date March 2021
 * 
 * @brief Application structures for the mapper
 */

#pragma once

#include "task.h"
#include "processor.h"

/* Forward declaration */
typedef struct _task task_t;

typedef struct _app {
	int id;
	unsigned task_cnt;
	unsigned allocated_cnt;
	unsigned mapping_score;
	task_t *task[PKG_MAX_TASKS_APP];
	bool has_static_tasks;
	int center_x;
	int center_y;
} app_t;

/**
 * @brief Initialize all apps
 * 
 * @param apps Pointer to the app array
 */
void app_init(app_t *apps);

/**
 * @brief Gets a unallocated app structure
 * 
 * @param apps Pointer to the app array
 * 
 * @return Pointer to a free app
 */
app_t *app_get_free(app_t *apps);

/**
 * @brief Builds an app received from the injector
 * 
 * @param app Pointer to the app structure
 * @param id Application ID
 * @param task_cnt Number of tasks
 * @param descriptor Pointer to the application descriptor
 * @param communication Pointer to the application communication list
 * @param tasks Pointer to the tasks array
 * @param processors Pointer to the processors array
 */
void app_build(app_t *app, int id, unsigned task_cnt, int *descriptor, int *communication, task_t *tasks, processor_t *processors);

/**
 * @brief Searches for a specific app
 * 
 * @param apps Pointer to the apps array
 * @param appid ID o the application to search
 * 
 * @return Pointer to the found app. NULL if not found.
 */
app_t *app_search(app_t *apps, int appid);

/**
 * @brief Gets the mapping order
 * 
 * @details The result is saved in the order parameter
 * 
 * @param app Pointer to the app
 * @param order Destination array of task pointers
 */
void app_get_order(app_t *app, task_t *order[]);
