#include <stddef.h>
#include <stdlib.h>

#include "list.h"

void list_init(list_t *list)
{
    list->head = NULL;
    list->tail = NULL;
}

list_entry_t *list_find(list_t *list, void *cmpval, bool (*find_fnc)(void*, void*))
{
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
    return entry->data;
}

void list_remove(list_t *list, list_entry_t *entry)
{
    if(entry == list->head)
        list->head = entry->next;

    if(entry == list->tail)
        list->tail = entry->prev;

    if(entry->prev)
        entry->prev->next = entry->next;

    if(entry->next)
        entry->next->prev = entry->prev;

    /* Remember to free dinamically allocated members of list entry */

    free(entry);
    entry = NULL;
}

void list_push_back(list_t *list, void *data)
{
    list_entry_t *entry = malloc(sizeof(list_entry_t));
    entry->data = data;
    entry->prev = list->tail;
    entry->next = NULL;

    if(list->tail != NULL)
        list->tail->next = entry;
    
    list->tail = entry;

    if(list->head == NULL)
        list->head = entry;
}
