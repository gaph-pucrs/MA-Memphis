/**
 * MA-Memphis
 * @file lru.c
 *
 * @author Angelo Elias Dal Zotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date September 2022
 *
 * @brief LRU queue for Memphis base on linked list
 */

#include "mutils/lru.h"

void lru_init(lru_t *lru, size_t max_size)
{
	list_init(&(lru->list));
	lru->max_size = max_size;
}

void *lru_use(lru_t *lru, void *cmpval, bool (*find)(void*, void*))
{
	list_entry_t *entry = list_find(&(lru->list), cmpval, find);
	if(entry == NULL)
		return NULL;

	void *data = list_get_data(entry);

	if(entry == list_back(&(lru->list)))
		return data;

	/* Remove entry from the list and add back in the end */
	list_remove(&(lru->list), entry);
	list_push_back(&(lru->list), data);

	return data;
}

void *lru_replace(lru_t *lru, void *data)
{
	void *old_data = NULL;
	if(lru->max_size == list_get_size(&(lru->list))){
		list_entry_t *entry = list_front(&(lru->list));
		old_data = list_get_data(entry);
		list_pop_front(&(lru->list));
	}

	list_push_back(&(lru->list), data);

	return old_data;
}
