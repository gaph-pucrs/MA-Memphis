#include "monitor.h"

#include "memphis.h"
#include "calls.h"

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
