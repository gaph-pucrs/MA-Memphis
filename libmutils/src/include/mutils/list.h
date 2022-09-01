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
#include <stddef.h>

/* Forward declaration */
typedef struct _list_entry list_entry_t;

/**
 * @brief Entry of the list
 */
struct _list_entry {
    list_entry_t *prev;
    list_entry_t *next;

    void *data;
};

/**
 * @brief List definitions 
 */
typedef struct _list {
    list_entry_t *head;
    list_entry_t *tail;
    size_t size;
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
 * @param cmpval Pointer to a value to pass to the custom find function or
 * pointer to stored data
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
 * @return list_entry_t* Pointer to the created entry, NULL if no memory
 */
list_entry_t *list_push_back(list_t *list, void *data);

/**
 * @brief Removes an element from list
 * 
 * @param list Pointer to the list structure
 * @param entry Pointer to the element to remove
 */
void list_remove(list_t *list, list_entry_t *entry);

/**
 * @brief Gets the first element of the list
 * 
 * @param list Pointer to the list structure
 * @return list_entry_t* Pointer to the first list entry
 */
list_entry_t *list_front(list_t *list);

/**
 * @brief Gets the last element of the list
 * 
 * @param list Pointer to the list structure
 * @return list_entry_t* Pointer to the last list entry
 */
list_entry_t *list_back(list_t *list);

/**
 * @brief Gets the next entry of a list
 * 
 * @param entry Pointer to an entry
 * @return list_entry_t* Pointer to the next element in the list
 */
list_entry_t *list_next(list_entry_t *entry);

/**
 * @brief Removes the first element of the list
 * 
 * @param list Pointer to the list
 */
void list_pop_front(list_t *list);

/**
 * @brief Checks if the list is empty
 * 
 * @param list Pointer to the list
 * @return true When empty
 * @return false When not empty
 */
bool list_empty(list_t *list);

/**
 * @brief Gets the number of elements in the list
 * 
 * @param list Pointer to the list
 * @return size_t Number of elements
 */
size_t list_get_size(list_t *list);

/**
 * @brief Creates a contiguous vector from the list
 * 
 * @param list Pointer to the list
 * @param data_size Size of each element in the list
 * @return void* Pointer to the created vector, NULL if no memory
 */
void *list_vectorize(list_t *list, size_t data_size);

/**
 * @brief Clears all elements inside a list
 * 
 * @details This does not frees the list itself
 * 
 * @param list Pointer to the list
 */
void list_clear(list_t *list);

/**
 * @brief Destroys all entries without clearing elements
 * 
 * @details This does not frees the list itself
 * 
 * @param list Pointer to the list
 */
void list_destroy(list_t *list);
