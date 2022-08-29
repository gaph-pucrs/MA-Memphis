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

#include "memphis.h"
#include "memphis/services.h"

#include "internal_syscall.h"

void monitor_init(volatile monitor_t *table)
{
	const int N_PE = memphis_get_nprocs(NULL, NULL);
	const int N_TASKS = memphis_get_max_tasks(NULL);

	for(int n = 0; n < N_PE; n++){
		for(int t = 0; t < N_TASKS; t++)
			table[n*N_TASKS + t].task = -1;
	}
}

int monitor_set_dmni(volatile monitor_t *table, enum MONITOR_TYPE type)
{
	return syscall_errno(SYS_monptr, 2, (long)table, type, 0, 0, 0, 0);
}

void monitor_announce(enum MONITOR_TYPE type)
{
	int16_t addr = memphis_get_addr();
	uint32_t payload = (addr << 16) | type;
	
	memphis_br_send_all(payload, ANNOUNCE_MONITOR);
}
