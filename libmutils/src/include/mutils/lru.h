/**
 * MA-Memphis
 * @file lru.h
 *
 * @author Angelo Elias Dal Zotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date September 2022
 *
 * @brief LRU queue for Memphis base on linked list
 */

#pragma once

#include "mutils/list.h"

/**
 * @brief LRU structure
 */
typedef struct _lru {
	list_t list;
	size_t max_size;
	size_t data_size;
} lru_t;

/**
 * @brief Initializes the LRU
 * 
 * @param lru Pointer to the LRU structure
 * @param max_size Maximum number of elements
 */
void lru_init(lru_t *lru, size_t max_size);

/**
 * @brief Fetch and update as most recently used
 * 
 * @param lru Pointer to LRU structure
 * @param cmpval Pointer of address to compare, or pointer to entry
 * @param find Function to find such as bool find(void *data, void *cmpval),
 * NULL to compare to entry pointer
 * 
 * @return void* Pointer to data in container
 */
void *lru_use(lru_t *lru, void *cmpval, bool (*find)(void*, void*));

/**
 * @brief Replace the least recently used data or create a new if has space
 * 
 * @param lru Pointer to the LRU structure
 * @param data Pointer to the new data to insert
 * 
 * @return void* Old data present in structure
 */
void *lru_replace(lru_t *lru, void *data);
