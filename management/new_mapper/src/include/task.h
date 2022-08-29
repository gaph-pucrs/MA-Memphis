/**
 * MA-Memphis
 * @file task.h
 *
 * @author Angelo Elias Dal Zotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date March 2021
 * 
 * @brief Task structures for the mapper
 */

#pragma once

#include <mutils/list.h>

#include "processing_element.h"
#include "window.h"

typedef enum _task_status {
	TASK_STATUS_BLOCKED,
	TASK_STATUS_RUNNING,
	TASK_STATUS_MIGRATING
} task_status_t;

typedef struct _task {
	int id;
	unsigned tag;

	pe_t *pe;

	list_t preds;
	list_t succs;

	task_status_t status;
} task_t;

void task_init(task_t *task, int appid, int taskid, unsigned tag);

int task_comm_push_back(task_t *vertex, task_t *succ);

int task_set_pe(task_t *task, pe_t *pe);

pe_t *task_get_pe(task_t *task);

list_t *task_get_succs(task_t *task);

list_t *task_get_preds(task_t *task);

list_t *task_get_order(task_t *task, list_t *order);

pe_t *task_map(task_t *task, pe_t *pes, wdo_t *window);

int task_get_id(task_t *task);
