/**
 * MA-Memphis
 * @file processing_element.h
 *
 * @author Angelo Elias Dal Zotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date March 2021
 * 
 * @brief Processor structures for the mapper
 */

#pragma once

#include <mutils/list.h>

/* Forward Declaration */
typedef struct _task task_t;

typedef struct _pe {
	int addr;
	unsigned slots;
	unsigned pending_cnt;
	list_t tasks;
} pe_t;

void pe_init(pe_t *pe, size_t slots, int addr);

int pe_add_pending(pe_t *pe);

list_entry_t *pe_task_push_back(pe_t *pe, task_t *task);

unsigned pe_get_slots(pe_t *pe);

list_t *pe_get_mapped(pe_t *pe);

int pe_get_addr(pe_t *pe);
