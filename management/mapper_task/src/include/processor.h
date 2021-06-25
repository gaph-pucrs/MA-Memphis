/**
 * MA-Memphis
 * @file processor.h
 *
 * @author Angelo Elias Dalzotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date March 2021
 * 
 * @brief Processor structures for the mapper
 */

#pragma once

#include <stdbool.h>

typedef struct _processor {
	int addr;
	int free_page_cnt;
	int pending_map_cnt;
	bool failed_map;
} processor_t;

/**
 * @brief Initializes the processors
 * 
 * @param List of processors
 */
void processor_init(processor_t *processors);

/**
 * @brief Gets a processor based on a X,Y addressing
 * 
 * @param processors List of processors
 * @param x X address
 * @param y Y address
 * 
 * @return Pointer to the desired processor
 */
processor_t *processors_get(processor_t *processors, int x, int y);
