/**
 * MA-Memphis
 * @file sliding_window.h
 *
 * @author Angelo Elias Dalzotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date June 2021
 * 
 * @brief Sliding window mapping heuristic
 */

#pragma once

#include "window.h"

/**
 * @brief Runs the sliding window mapper heuristic
 * 
 * @param app Pointer to the app to map
 * @param processors List of processors
 */
void sw_map_app(app_t *app, processor_t *processors);

/**
 * @brief Map the dynamic tasks with the sliding window heuristic
 * 
 * @param app Pointer to the app to map
 * @param order Array of task pointers in the correct mapping order
 * @param processors List of processors
 * @param window Pointer to the mapping window
 */
void sw_map_dynamic(app_t *app, task_t *order[], processor_t *processors, window_t *window);

/**
 * @brief Gets the best PE to map a task based on the sliding window heuristic
 * 
 * @param task Pointer to the task to map
 * @param app Pointer to the app to map
 * @param processors List of processors
 * @param window Pointer to the mapping window
 * 
 * @return Sequential address of the PE to map the task
 */
int sw_map_task(task_t *task, app_t *app, processor_t *processors, window_t *window);
