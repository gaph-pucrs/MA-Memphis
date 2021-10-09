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

#include "monitor.h"

#include "memphis.h"
#include "services.h"

void monitor_init(volatile monitor_t table[PKG_N_PE][PKG_MAX_LOCAL_TASKS])
{
	for(int n = 0; n < PKG_N_PE; n++){
		for(int t = 0; t < PKG_MAX_LOCAL_TASKS; t++){
			table[n][t].task = -1;
		}
	}
}

bool monitor_set_dmni(volatile monitor_t table[PKG_N_PE][PKG_MAX_LOCAL_TASKS], enum MONITOR_TYPE type)
{
	return !system_call(SCALL_MON_PTR, table, type, 0);
}

void monitor_announce(enum MONITOR_TYPE type)
{
	int16_t addr = memphis_get_addr();
	uint32_t payload = (addr << 16) | type;
	
	memphis_br_send_all(payload, ANNOUNCE_MONITOR);
}
