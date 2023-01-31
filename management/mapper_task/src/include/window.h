/**
 * MA-Memphis
 * @file window.h
 *
 * @author Angelo Elias Dal Zotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date June 2021
 * 
 * @brief Window structures for the sliding window mapping heuristic
 */

#pragma once

#include <stdbool.h>

#include "processing_element.h"

typedef struct _wdo {
	int x;
	int y;

	int wx;
	int wy;
} wdo_t;

void wdo_init(wdo_t *window, int x, int y, int wx, int wy);

void wdo_from_center(wdo_t *window, pe_t *pes, int req_slots);

bool wdo_has_slots(wdo_t *window, pe_t *pes, int req_slots);

void wdo_slide(wdo_t *window);

void wdo_from_last(wdo_t *window, pe_t *pes, int req_slots);
