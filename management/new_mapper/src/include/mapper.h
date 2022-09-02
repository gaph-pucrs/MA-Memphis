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

#include "application.h"
#include "processing_element.h"

#define MAP_DESCR_ENTRY_LEN 2
#define MAP_STRIDE 2
#define MAP_MIN_WX 3
#define MAP_MIN_WY 3

typedef struct _map {
	unsigned appid_cnt;
	size_t slots;
	app_t *pending;
	pe_t *pes;
	
	list_t apps;
} map_t;

void map_init(map_t *mapper);

void map_new_app(map_t *mapper, int injector, size_t task_cnt, int *descriptor, int *communication);

int map_coord_to_idx(int coord);

int map_idx_to_coord(int idx);

int map_xy_to_idx(int x, int y);

unsigned map_manhattan(int a, int b);

void map_task_allocated(map_t *mapper, int id);

void map_task_terminated(map_t *mapper, int id);

void map_task_aborted(map_t *mapper, int id);

void map_request_service(map_t *mapper, int address, unsigned tag, int requester);

void map_migration_map(map_t *mapper, int id);
