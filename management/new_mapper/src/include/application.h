/**
 * MA-Memphis
 * @file application.h
 *
 * @author Angelo Elias Dal Zotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date March 2021
 * 
 * @brief Application structures for the mapper
 */

#pragma once

#include "task.h"

typedef struct _app {
	int id;
	unsigned failed_cnt;
	unsigned allocated_cnt;
	size_t task_cnt;
	task_t *tasks;

	float score;
	bool has_static;
} app_t;
