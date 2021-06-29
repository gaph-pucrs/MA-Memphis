/**
 * MA-Memphis
 * @file mapper.h
 *
 * @author Angelo Elias Dalzotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date March 2021
 * 
 * @brief Main mapper functions
 */

#pragma once

#include "window.h"

#define TASK_DESCRIPTOR_SIZE 2

#define MAP_STRIDE 2
#define MAP_MIN_WX 3
#define MAP_MIN_WY 3

typedef struct _mapper {
	unsigned pending_task_cnt;

	int pending_descr[PKG_MAX_TASKS_APP * 2];
	int pending_comm[PKG_MAX_TASKS_APP*(PKG_MAX_TASKS_APP - 1)];
	app_t *pending_map_app;

	unsigned fail_map_cnt;

	int available_slots;
	int appid_cnt;
	app_t apps[PKG_MAX_LOCAL_TASKS*PKG_N_PE];
	task_t tasks[PKG_MAX_LOCAL_TASKS*PKG_N_PE];
	processor_t processors[PKG_N_PE];
} mapper_t;

/**
 * @brief Initializes the mapper
 * 
 * @param mapper Pointer to the mapper structure
 */
void map_init(mapper_t *mapper);

/**
 * @brief Handles a NEW_APP received
 * 
 * @param mapper Pointer to the mapper structure
 * @param task_cnt Number of tasks of the new app
 * @param descriptor Pointer to the application descriptor
 * @param communication Pointer to the application communication list
 */
void map_new_app(mapper_t *mapper, unsigned task_cnt, int *descriptor, int *communication);

/**
 * @brief Check if it is possible to map all the static tasks
 * 
 * @param app Pointer to the app
 * @param processors Pointer to the processors list
 * 
 * @return Number of PEs that couldn't receive static mapped tasks
 */
unsigned map_try_static(app_t *app, processor_t *processors);

/**
 * @brief Applies the mapping of static tasks
 * 
 * @details The mapping should be checked first by map_try_static function
 * 
 * @param app Pointer to the app structure
 * @param processors Pointer to the list of processors
 */
void map_static_tasks(app_t *app, processor_t *processors);

/**
 * @brief Sets the mapping score
 * 
 * @details Mean manhattan distance of communicating tasks
 * 
 * @param app Pointer to app structure
 * @param processors List of processors
 */
void map_set_score(app_t *app, processor_t *processors);

/**
 * @brief Handles a TASK_ALLOCATED message
 * 
 * @param mapper Pointer to mapper structure
 * @param id ID of the allocated task
 */
void map_task_allocated(mapper_t *mapper, int id);

/**
 * @brief Handles a TASK_TERMINATED message
 * 
 * @param mapper Pointer to the mapper structure
 * @param id ID of the terminated task
 */
void map_task_terminated(mapper_t *mapper, int id);

/**
 * @brief Sends a TASK_ALLOCATION message to the injectore
 * 
 * @param app Pointer to the app that will be allocated
 * @param processors List of processors
 */
void map_task_allocation(app_t *app, processor_t *processors);

/**
 * @brief Tries mapping the application
 * 
 * @param mapper Pointer to the mapper structure
 * @param appid ID of the application
 * @param task_cnt Number of tasks in the application
 * @param descr Pointer to the application descriptor
 * @param comm Pointer to the application communication list
 * @param processors List of processors
 */
void map_try_mapping(mapper_t *mapper, int appid, int task_cnt, int *descr, int *comm, processor_t *processors);

/**
 * @brief Sends TASK_RELEASE messages
 * 
 * @param mapper Pointer to the mapper structure
 * @param app Pointer to the app to release
 */
void map_task_release(mapper_t *mapper, app_t *app);

/**
 * @brief Sends an APP_MAPPING_COMPLETE message to the injector
 * 
 * @param app Pointer to the app that has completed the mappign
 */
void map_app_mapping_complete(app_t *app);

/**
 * @brief Searches the nearest task with a task type tag
 * 
 * @param mapper Pointer to the mapper structure
 * @param ma Pointer to the MA app (app 0)
 * @param address Address of the requester
 * @param tag Desired tag to search
 * 
 * @return Pointer to the found task
 */
task_t *map_nearest_tag(mapper_t *mapper, app_t *ma, int address, unsigned tag);

/**
 * @brief Gets the manhattan distance between two points
 * 
 * @param source First address
 * @param target Second address
 * 
 * @return The manhattan distance
 */
unsigned map_manhattan_distance(int source, int target);

/**
 * @brief Handles a REQUEST_SERVICE message
 * 
 * @param mapper Pointer to the mapper structure
 * @param address Address of the requester
 * @param tag Task type tag requested
 * @param requester The requester task ID
 */
void map_request_service(mapper_t *mapper, int address, unsigned tag, int requester);
