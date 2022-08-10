/**
 * MA-Memphis
 * @file list.h
 *
 * @author Angelo Elias Dal Zotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date August 2022
 *
 * @brief Linked list for Memphis
 */

#pragma once

#include <stdbool.h>

typedef struct _list_entry list_entry_t;

struct _list_entry {
    list_entry_t *prev;
    list_entry_t *next;

    void *data;
};

typedef struct _list {
    list_entry_t *head;
    list_entry_t *tail;
} list_t;

/**
 * @brief Initializes a list
 * 
 * @param list Pointer to list structure
 */
void list_init(list_t *list);

/**
 * @brief Finds an item in the list
 * 
 * @param list Pointer to the list structure
 * @param cmpval Pointer to a value to pass to the custom find function
 * @param find Pointer to the find function that receives a pointer to the stored 
 * data and another pointer to the value to find (find(entry->data, cmpval)). If
 * NULL, it will compare the pointer of the entry->data to cmpval.
 * 
 * @return list_entry_t* Pointer to the found entry
 */
list_entry_t *list_find(list_t *list, void *cmpval, bool (*find)(void*, void*));

/**
 * @brief Gets the pointer to stored data
 * 
 * @param entry Pointer to list entry
 * 
 * @return void* Pointer to data
 */
void *list_get_data(list_entry_t *entry);

/**
 * @brief Adds an element to the tail of the list
 * 
 * @param list Pointer to list structure
 * @param data Pointer to data to store in list
 */
void list_push_back(list_t *list, void *data);

/**
 * @brief Removes an element from list
 * 
 * @param list Pointer to the list structure
 * @param entry Pointer to the element to remove
 */
void list_remove(list_t *list, list_entry_t *entry);
