/**
 * MA-Memphis
 * @file list.c
 *
 * @author Angelo Elias Dal Zotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date August 2022
 *
 * @brief Linked list for Memphis
 */

#include "mutils/list.h"

#include <stdlib.h>
#include <string.h>

void list_init(list_t *list)
{
	list->head = NULL;
	list->tail = NULL;
	list->size = 0;
}

list_entry_t *list_find(list_t *list, void *cmpval, bool (*find_fnc)(void*, void*))
{
	if(list == NULL)
		return NULL;
		
	list_entry_t *current = list->head;
	while(current != NULL){
		if(
			(find_fnc == NULL && current->data == cmpval) ||
			(find_fnc != NULL && find_fnc(current->data, cmpval))
		)
			return current;

		current = current->next;
	}

	return NULL;
}

void *list_get_data(list_entry_t *entry)
{
	if(entry == NULL)
		return NULL;

	return entry->data;
}

list_entry_t *list_push_back(list_t *list, void *data)
{
	if(list == NULL)
		return NULL;

	list_entry_t *entry = malloc(sizeof(list_entry_t));

	if(entry == NULL)
		return NULL;

	entry->data = data;
	entry->prev = list->tail;
	entry->next = NULL;

	if(list->tail != NULL)
		list->tail->next = entry;
	
	list->tail = entry;

	if(list->head == NULL)
		list->head = entry;

	list->size++;

	return entry;
}

void list_remove(list_t *list, list_entry_t *entry)
{
	if(list == NULL || entry == NULL)
		return;

	if(entry == list->head)
		list->head = entry->next;

	if(entry == list->tail)
		list->tail = entry->prev;

	if(entry->prev != NULL)
		entry->prev->next = entry->next;

	if(entry->next != NULL)
		entry->next->prev = entry->prev;

	/* Remember to free dinamically allocated members of list entry */

	free(entry);

	list->size--;
}

list_entry_t *list_front(list_t *list)
{
	if(list == NULL)
		return NULL;

	return list->head;
}

list_entry_t *list_back(list_t *list)
{
	if(list == NULL)
		return NULL;

	return list->tail;
}

list_entry_t *list_next(list_entry_t *entry)
{
	if(entry == NULL)
		return NULL;

	return entry->next;
}

void list_pop_front(list_t *list)
{
	if(list == NULL)
		return;

	list_entry_t *entry = list->head;

	if(entry == NULL)
		return;

	list->head = entry->next;

	if(entry == list->tail)
		list->tail = NULL;

	if(entry->next != NULL)
		entry->next->prev = NULL;

	free(entry);

	list->size--;
}

bool list_empty(list_t *list)
{
	if(list == NULL)
		return true;

	return (list->head == NULL);
}

size_t list_get_size(list_t *list)
{
	if(list == NULL)
		return 0;

	return list->size;
}

void *list_vectorize(list_t *list, size_t data_size)
{
	if(list == NULL || list->size == 0)
		return NULL;

	list_entry_t *entry = list->head;

	if(entry == NULL)
		return NULL;

	void *vector = malloc((list->size) * data_size);

	if(vector == NULL)
		return NULL;
	
	void *ptr = vector;

	while(entry != NULL){
		memcpy(ptr, entry->data, data_size);
		ptr += data_size;
		entry = entry->next;
	}

	return vector;
}

void list_clear(list_t *list)
{
	if(list == NULL)
		return;

	list_entry_t *entry = list->head;

	while(entry != NULL){
		free(entry->data);
		list_entry_t *oldentry = entry;
		entry = entry->next;
		free(oldentry);
	}
}

void list_destroy(list_t *list)
{
	list_entry_t *entry = list->head;

	while(entry != NULL){
		list_entry_t *oldentry = entry;
		entry = entry->next;
		free(oldentry);
	}
}
