/**
 * MA-Memphis
 * @file processing_element.c
 *
 * @author Angelo Elias Dal Zotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date March 2021
 * 
 * @brief Processor structures for the mapper
 */

#include "processing_element.h"

#include <stdio.h>

#include "task.h"

void pe_init(pe_t *pe, size_t slots, int addr)
{
	pe->addr = addr;
	pe->pending_cnt = 0;
	pe->slots = slots;
	list_init(&(pe->tasks));
}

bool pe_add_pending(pe_t *pe)
{
	pe->pending_cnt++;
	return (pe->pending_cnt > pe->slots);
}

list_entry_t *pe_task_push_back(pe_t *pe, task_t *task)
{
	pe->pending_cnt--;
	pe->slots--;
	return list_push_back(&(pe->tasks), task);
}

unsigned pe_get_slots(pe_t *pe)
{
	return pe->slots;
}

list_t *pe_get_mapped(pe_t *pe)
{
	return &(pe->tasks);
}

int pe_get_addr(pe_t *pe)
{
	return pe->addr;
}

bool pe_task_remove(pe_t *pe, task_t *task)
{
	list_entry_t *entry = list_find(&(pe->tasks), task, NULL);
	if(entry == NULL)
		return false;

	list_remove(&(pe->tasks), entry);

	bool ret = (pe->pending_cnt > pe->slots);

	pe->slots++;

	return ret;
}
