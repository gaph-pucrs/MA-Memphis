/**
 * MA-Memphis
 * @file application.h
 *
 * @author Angelo Elias Dal Zotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date August 2022
 *
 * @brief Control location of communicating tasks
 */

#pragma once

#include <stddef.h>

/**
 * @brief Application structure
 * 
 * @details This is used to store location of communicating tasks 
 */
typedef struct _app {
    int id;
    unsigned ref_cnt;
    size_t task_cnt;
    int *task_location;
} app_t;

/**
 * @brief Initializes a list of applications
 */
void app_init();

/**
 * @brief Finds an application inside the applications list
 * 
 * @param appid ID of the application (task id >> 8)
 * 
 * @return app_t* Pointer to the application
 */
app_t *app_find(int appid);

/**
 * @brief Creates an application and inserts into the applications list
 * 
 * @param appid ID of the application to create (task id >> 8)
 * 
 * @return app_t* Pointer to the created application
 */
app_t *app_emplace_back(int appid);

/**
 * @brief Add to the app reference counter
 * 
 * @param app Pointer to the app
 */
void app_refer(app_t *app);

/**
 * @brief Remove from the app reference counter
 * 
 * @param app Pointer to the app
 */
void app_derefer(app_t *app);

/**
 * @brief Gets address of an application task
 * 
 * @param app Pointer to the app structure
 * @param task ID of the task
 * @return int Address of the task
 */
int app_get_address(app_t *app, int task);

/**
 * @brief Updates the location of a task
 * 
 * @param app Pointer to the application
 * @param task ID of the task
 * @param addr Address of the task
 */
void app_update(app_t *app, int task, int addr);

/**
 * @brief Sets the task locations of the application from a pointer
 * 
 * @param app Pointer to the application
 * @param task_cnt Number of tasks
 * @param task_location Pointer to set
 * @return int task_cnt on success
 */
void app_set_location(app_t *app, size_t task_cnt, int *task_location);

/**
 * @brief Copies the task location of the application
 * 
 * @param app Pointer to the app
 * @param task_cnt Number of tasks
 * @param task_location Pointer to copy from
 * @return int task_cnt on success
 */
int app_copy_location(app_t *app, size_t task_cnt, int *task_location);

/**
 * @brief Get the number of tasks of the application
 * 
 * @param app Pointer to the app
 * @return size_t Number of tasks
 */
size_t app_get_task_cnt(app_t *app);

/**
 * @brief Get the array of task locations from an app
 * 
 * @param app Pointer to the application
 * @return int* Pointer to the array of locations
 */
int *app_get_locations(app_t *app);
