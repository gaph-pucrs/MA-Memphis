#include "window.h"
#include "mapper.h"

void window_set_from_center(processor_t *processors, app_t *app, window_t *window, unsigned req_pages, int wx, int wy, bool raise_x)
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

	while(!window_has_pages(processors, window, req_pages)){
		/* Select window by changing W instead of sliding */		
		if(raise_x){
			window->wx++;
			raise_x = false;
		} else {
			window->wy++;
			raise_x = true;
		}

		if(window->x + window->wx > PKG_N_PE_X)
			window->x = PKG_N_PE_X - window->wx;
		
		if(window->y + window->wy > PKG_N_PE/PKG_N_PE_X)
			window->y = PKG_N_PE/PKG_N_PE_X - window->wy;
	}
}

bool window_has_pages(processor_t *processors, window_t *window, int req_pages)
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
