/**
 * MA-Memphis
 * @file window.h
 *
 * @author Angelo Elias Dalzotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date June 2021
 * 
 * @brief Window structures for the sliding window mapping heuristic
 */

#pragma once

#include <stdbool.h>

#include "processor.h"
#include "app.h"

typedef struct _window {
	int x;
	int y;

	int wx;
	int wy;
} window_t;

/**
 * @brief Searches for a window where an application can be mapped
 * 
 * @param window Pointer to a window structure
 * @param processors Array of processors
 * @param app Pointer to the app to map in the window
 */
void window_search(window_t *window, processor_t *processors, app_t *app);

/**
 * @brief Searches a window to map based on a central position
 * 
 * @details The desired center should be set inside the app structure
 * 
 * @param window Pointer to the window
 * @param processors Array of processors
 * @param app Pointer to the app to map in the window
 * @param req_pages Number of available pages to search for in the window
 * @param wx Starting size of the window in X
 * @param wy Starting size of the window in Y
 */
void window_set_from_center(window_t *window, processor_t *processors, app_t *app, unsigned req_pages, int wx, int wy);

/**
 * @brief Searches a window to map based on the last searched window
 * 
 * @param window Pointer to the window
 * @param processors Array of processors
 * @param app Pointer to the app to map in the window
 * @param req_pages Number of available pages to search for in the window
 * @param wx Starting size of the window in X
 * @param wy Starting size of the window in Y
 */
void window_set_from_last(window_t *window, processor_t *processors, app_t *app, unsigned req_pages, int wx, int wy);

/**
 * @brief Iterates the window
 * 
 * @details This is based on the many-core size and the stride
 * 
 * @param window Pointer to the window
 */
void window_next(window_t *window);

/**
 * @brief Verifies if the window has a minimum number of pages available
 * 
 * @param window Pointer to the window
 * @param processors Array of processors
 * @param req_pages Number of required available pages
 * 
 * @return True if the window has the req_pages
 */
bool window_has_pages(window_t *window, processor_t *processors, int req_pages);
