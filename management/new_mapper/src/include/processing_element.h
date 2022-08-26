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

typedef struct _pe {
	int addr;
	unsigned slots;
	unsigned pending_cnt;
	list_t tasks;
} pe_t;

void pe_init(pe_t *pe, int addr);
