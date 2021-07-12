/**
 * MA-Memphis
 * @file window.c
 *
 * @author Angelo Elias Dalzotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date June 2021
 * 
 * @brief Window structures for the sliding window mapping heuristic
 */

#include "window.h"
#include "mapper.h"

void window_search(window_t *window, processor_t *processors, app_t *app)
{
	int wx = MAP_MIN_WX;
	int wy = MAP_MIN_WY;

	while(wx*wy*PKG_MAX_LOCAL_TASKS < app->task_cnt){
		if(wy <= wx){	/* Prefer to have y greater than x because window slides in x-axis */
			wy++;
		} else {
			wx++;
		}
	}

	if(wx > PKG_N_PE_X)
		wx = PKG_N_PE_X;

	if(wy > PKG_N_PE/PKG_N_PE_X)
		wy = PKG_N_PE/PKG_N_PE_X;

	if(app->has_static_tasks){
		/* Select a window without changing last_window */
		window_set_from_center(window, processors, app, app->task_cnt, wx, wy);
	} else {
		/* Select a window based on the last window chosen */
		window_set_from_last(window, processors, app, app->task_cnt, wx, wy);
	}
}

void window_set_from_center(window_t *window, processor_t *processors, app_t *app, unsigned req_pages, int wx, int wy)
{
	/* Window will be based on the center of the static tasks */
	window->x = app->center_x - wx / 2;
	window->y = app->center_y - wy / 2;
	window->wx = wx;
	window->wy = wy;

	if(window->x < 0)
		window->x = 0;
	
	if(window->y < 0)
		window->y = 0;

	if(window->x + window->wx > PKG_N_PE_X)
		window->x = PKG_N_PE_X - window->wx;
	
	if(window->y + window->wy > PKG_N_PE/PKG_N_PE_X)
		window->y = PKG_N_PE/PKG_N_PE_X - window->wy;

	while(!window_has_pages(window, processors, req_pages)){
		/* Select window by changing W instead of sliding */		
		if(window->wy <= window->wx){
			window->wy++;
		} else {
			window->wx++;
		}

		if(window->x + window->wx > PKG_N_PE_X)
			window->x = PKG_N_PE_X - window->wx;
		
		if(window->y + window->wy > PKG_N_PE/PKG_N_PE_X)
			window->y = PKG_N_PE/PKG_N_PE_X - window->wy;
	}
}

void window_set_from_last(window_t *window, processor_t *processors, app_t *app, unsigned req_pages, int wx, int wy)
{
	static int last_x = PKG_N_PE_X - MAP_MIN_WX;
	static int last_y = PKG_N_PE/PKG_N_PE_X - MAP_MIN_WY;

	if(wx != MAP_MIN_WX || wy != MAP_MIN_WY){
		last_x = PKG_N_PE_X - wx;
		last_y = PKG_N_PE/PKG_N_PE_X - wy;
	}
	// printf("LW %dx%d\n", last_x, last_y);
	// printf("SS %dx%d\n", wx, wy);
	// printf("Starting window size is %dx%d\n", last_window.wx, last_window.wy);

	window->x = last_x;
	window->y = last_y;
	window->wx = wx;
	window->wy = wy;
	window_next(window);

	while(true){
		/* From last window to top right corner */
		while(window->x > last_x || window->y > last_y){

			if(window_has_pages(window, processors, app->task_cnt)){
				last_x = window->x;
				last_y = window->y;
				return;
			}

			window_next(window);
		}

		/* From bottom left corner to last window */
		while(window->x < last_x || window->y < last_y){

			if(window_has_pages(window, processors, app->task_cnt)){
				last_x = window->x;
				last_y = window->y;
				return;
			}

			window_next(window);
		}

		/* Exactly last window */
		if(window_has_pages(window, processors, app->task_cnt)){
			last_x = window->x;
			last_y = window->y;
			return;
		}

		/* No window found */
		if(window->wy <= window->wx){
			window->wy++;
		} else {
			window->wx++;
		}
		last_x = PKG_N_PE_X - window->wx;
		last_y = PKG_N_PE/PKG_N_PE_X - window->wy;
		// printf("CS %dx%d\n", window->wx, window->wy);

		window->x = last_x;
		window->y = last_y;
		window_next(window);
	}
}

void window_next(window_t *window)
{
	if(window->x + window->wx < PKG_N_PE_X){
		window->x += MAP_STRIDE;

		if(window->x + window->wx > PKG_N_PE_X)
			window->x = PKG_N_PE_X - window->wx;
	} else if(window->y + window->wy < PKG_N_PE/PKG_N_PE_X){
		window->y += MAP_STRIDE;
		window->x = 0;

		if(window->y + window->wy > PKG_N_PE/PKG_N_PE_X)
			window->y = PKG_N_PE/PKG_N_PE_X - window->wy;
	} else {
		window->x = 0;
		window->y = 0;
	}
}

bool window_has_pages(window_t *window, processor_t *processors, int req_pages)
{
	for(int x = window->x; x < window->x + window->wx; x++){
		for(int y = window->y; y < window->y + window->wy; y++){
			int seq = x + y*PKG_N_PE_X;
			req_pages -= processors[seq].free_page_cnt;
			if(req_pages <= 0)
				return true;
		}
	}
	return false;
}
