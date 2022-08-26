/**
 * MA-Memphis
 * @file window.c
 *
 * @author Angelo Elias Dal Zotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date June 2021
 * 
 * @brief Window structures for the sliding window mapping heuristic
 */

#include "window.h"

extern size_t _PE_X_CNT;
extern size_t _PE_Y_CNT;

void wdo_init(wdo_t *window, int x, int y, int wx, int wy)
{
	window->x = x;
	window->y = y;
	window->wx = wx;
	window->wy = wy;
}

void wdo_from_center(wdo_t *window, pe_t *pes, int req_slots)
{
	if(window->x < 0)
		window->x = 0;
	
	if(window->y < 0)
		window->y = 0;

	if(window->x + window->wx > _PE_X_CNT)
		window->x = _PE_X_CNT - window->wx;
	
	if(window->y + window->wy > _PE_Y_CNT)
		window->y = _PE_Y_CNT - window->wy;

	while(!wdo_has_slots(window, pes, req_slots)){
		/* Select window by changing W instead of sliding */		
		if(window->wy <= window->wx){
			window->wy++;
		} else {
			window->wx++;
		}

		if(window->x + window->wx > _PE_X_CNT)
			window->x = _PE_X_CNT - window->wx;
		
		if(window->y + window->wy > _PE_Y_CNT)
			window->y = _PE_Y_CNT - window->wy;
	}
}

bool wdo_has_pages(wdo_t *window, pe_t *pes, int req_slots)
{
	for(int x = window->x; x < window->x + window->wx; x++){
		for(int y = window->y; y < window->y + window->wy; y++){
			int seq = map_xy_to_idx(x, y);
			req_slots -= pes[seq].slots;
			if(req_slots <= 0)
				return true;
		}
	}
	return false;
}

void wdo_slide(wdo_t *window)
{
	if(window->x + window->wx < _PE_X_CNT){
		window->x += MAP_STRIDE;

		if(window->x + window->wx > _PE_X_CNT)
			window->x = _PE_X_CNT - window->wx;
	} else if(window->y + window->wy < _PE_Y_CNT){
		window->y += MAP_STRIDE;
		window->x = 0;

		if(window->y + window->wy > _PE_Y_CNT)
			window->y = _PE_Y_CNT - window->wy;
	} else {
		window->x = 0;
		window->y = 0;
	}
}

void wdo_from_last(wdo_t *window, pe_t *pes, int req_slots)
{
	while(true){
		/* From last window to top right corner */
		while(window->x > last_x || window->y > last_y){

			if(wdo_has_pages(window, pes, req_slots)){
				last_x = window->x;
				last_y = window->y;
				return;
			}

			wdo_slide(window);
		}

		/* From bottom left corner to last window */
		while(window->x < last_x || window->y < last_y){

			if(wdo_has_pages(window, pes, req_slots)){
				last_x = window->x;
				last_y = window->y;
				return;
			}

			wdo_slide(window);
		}

		/* Exactly last window */
		if(wdo_has_pages(window, pes, req_slots)){
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
		last_x = _PE_X_CNT - window->wx;
		last_y = _PE_Y_CNT - window->wy;
		// printf("CS %dx%d\n", window->wx, window->wy);

		window->x = last_x;
		window->y = last_y;
		wdo_slide(window);
	}
}