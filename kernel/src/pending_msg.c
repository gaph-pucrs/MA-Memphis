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

#include "pending_msg.h"

#include <stdlib.h>

#include <memphis.h>
#include <mutils/list.h>

#include "mmr.h"

list_t _pmsgs;

void pmsg_init()
{
	list_init(&_pmsgs);
}

int pmsg_emplace_back(void *buf, size_t size, int cons_task)
{
	opipe_t *opipe = malloc(sizeof(opipe_t));

	if(opipe == NULL)
		return -1;

	int result = opipe_push(
		opipe, 
		buf, 
		size, 
		MEMPHIS_KERNEL_MSG | MMR_NI_CONFIG, 
		cons_task
	);

	if(result != size)
		return result;

	if(list_push_back(&_pmsgs, opipe) == NULL)
		return -1;

	return size;
}

bool _pmsg_find_fnc(void *data, void *cmpval)
{
	opipe_t *opipe = (opipe_t*)data;
	int cons_task = *((int*)cmpval);

	return (opipe->consumer_task == cons_task);
}

opipe_t *pmsg_find(int cons_task)
{
	list_entry_t *entry = list_find(&_pmsgs, &cons_task, _pmsg_find_fnc);

	if(entry == NULL)
		return NULL;

	return list_get_data(entry);
}

void pmsg_remove(opipe_t *pending)
{
	list_entry_t *entry = list_find(&_pmsgs, pending, NULL);

	if(entry != NULL)
		list_remove(&_pmsgs, entry);

	free(pending);
}
