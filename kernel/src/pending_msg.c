/**
 * MA-Memphis
 * @file pending_msg.c
 *
 * @author Angelo Elias Dal Zotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date August 2022
 *
 * @brief Outgoing kernel messages encapsulated in the Memphis messaging API.
 */

#include <stdlib.h>

#include <memphis.h>

#include "list.h"
#include "pending_msg.h"
#include "mmr.h"

list_t _pend_msgs;

void pend_msg_init()
{
    list_init(&_pend_msgs);
}

int pend_msg_push(void *buf, size_t size, int cons_task)
{
    opipe_t *opipe = malloc(sizeof(opipe));
    int result = opipe_push(
        opipe, 
        buf, 
        size, 
        MEMPHIS_KERNEL_MSG | MMR_NI_CONFIG, 
        cons_task
    );

    if(result != size)
        return result;

    list_push_back(&_pend_msgs, opipe);

	return size;
}

bool _pend_msg_find_fnc(void *data, void *cmpval)
{
    opipe_t *opipe = (opipe_t*)data;
    int cons_task = *((int*)cmpval);

    return (opipe->consumer_task == cons_task);
}

opipe_t *pend_msg_find(int cons_task)
{
    list_entry_t *entry = list_find(&_pend_msgs, &cons_task, _pend_msg_find_fnc);

    if(entry == NULL)
        return NULL;

    return list_get_data(entry);
}

void pend_msg_remove(opipe_t *pending)
{
    list_entry_t *entry = list_find(&_pend_msgs, pending, NULL);
    list_remove(&_pend_msgs, entry);
}
