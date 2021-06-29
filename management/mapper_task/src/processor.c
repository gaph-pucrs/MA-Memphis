/**
 * MA-Memphis
 * @file processor.c
 *
 * @author Angelo Elias Dalzotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date March 2021
 * 
 * @brief Processor structures for the mapper
 */

#include "processor.h"

void processor_init(processor_t *processors)
{
	for(int i = 0; i < PKG_N_PE; i++){
		processors[i].addr = i % PKG_N_PE_X << 8 | i / PKG_N_PE_X;
		processors[i].free_page_cnt = PKG_MAX_LOCAL_TASKS;
		processors[i].pending_map_cnt = 0;
		processors[i].failed_map = false;
	}
}

processor_t *processors_get(processor_t *processors, int x, int y)
{
	int seq = x + y*PKG_N_PE_X;
	return &processors[seq];
}
