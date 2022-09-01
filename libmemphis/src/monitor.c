/**
 * MA-Memphis
 * @file monitor.c
 * 
 * @author Angelo Elias Dalzotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date September 2021
 * 
 * @brief Monitoring API using the BrNoC
 */

#include "memphis/monitor.h"

#include <stdlib.h>

#include "memphis.h"
#include "memphis/services.h"

#include "internal_syscall.h"

mon_t *mon_create(size_t *slots)
{
	memphis_get_max_tasks(slots);

	mon_t *table = malloc(sizeof(mon_t) * (*slots));

	if(table == NULL)
		return NULL;

	for(int i = 0; i < *slots; i++)
			table[i].task = -1;

	return table;
}

int mon_set_dmni(mon_t *table, enum MONITOR_TYPE type)
{
	return syscall_errno(SYS_monptr, 2, (long)table, type, 0, 0, 0, 0);
}

void mon_announce(enum MONITOR_TYPE type)
{
	int16_t addr = memphis_get_addr();
	uint32_t payload = (addr << 16) | type;
	
	memphis_br_send_all(payload, ANNOUNCE_MONITOR);
}
