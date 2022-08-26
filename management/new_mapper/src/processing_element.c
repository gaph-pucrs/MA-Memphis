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

extern unsigned _PE_SLOTS;

void pe_init(pe_t *pe, int addr)
{
	pe->addr = addr;
	pe->pending_cnt = 0;
	pe->slots = _PE_SLOTS;
	list_init(&(pe->tasks));
}

int pe_add_pending(pe_t *pe)
{
	pe->pending_cnt++
	return (pe->pending_cnt > pe->slots);
}

list_entry_t *pe_task_push_back(pe_t *pe, task_t *task)
{
	pe->pending_cnt--;
	return list_push_back(&(pe->tasks), task);
}

unsigned pe_get_slots(pe_t *pe)
{
	return pe->slots;
}
