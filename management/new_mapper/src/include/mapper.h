/**
 * MA-Memphis
 * @file mapper.h
 *
 * @author Angelo Elias Dal Zotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date March 2021
 * 
 * @brief Main mapper functions
 */

#pragma once

#include "processing_element.h"

#define MAP_DESCR_ENTRY_LEN 2
#define MAP_STRIDE 2
#define MAP_MIN_WX 3
#define MAP_MIN_WY 3

typedef struct _map {
	unsigned appid_cnt;
	app_t *pending;
	
	pe_t *pes;
} map_t;

void map_init(map_t *mapper);

void map_new_app(map_t *map, size_t task_cnt, int *descriptor, int *communication);
