/**
 * 
 * @file llm.c
 *
 * @author Angelo Elias Dalzotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date March 2020
 * 
 * @brief Implements the Low-Level Monitor for Management Application support.
 */

#include "llm.h"

#include "syscall.h"
#include "broadcast.h"
#include "monitor.h"
#include "interrupts.h"

#include "services.h"

void llm_clear_table(tcb_t *task)
{
	int id = task->id & 0xFFFF;
	os_clear_mon_table(id);

	uint32_t message = (CLEAR_MON_TABLE << 16) | (id);
	while(!br_send(message, MMR_NI_CONFIG, MMR_NI_CONFIG, BR_SVC_ALL));
}

void llm_rt(tcb_t *tasks)
{
	static unsigned last_rt[PKG_MAX_LOCAL_TASKS];
	unsigned now = MMR_TICK_COUNTER;

	for(int i = 0; i < PKG_MAX_LOCAL_TASKS; i++){
		if(now - last_rt[i] >= PKG_SLACK_TIME_WINDOW/10){ /* Update 10 times faster than the real time observer */
			if(tasks[i].id != -1 && tasks[i].observer_task != -1 && tasks[i].scheduler.deadline != -1 && !tasks[i].scheduler.waiting_msg && tasks[i].proc_to_migrate == -1){
				int payload = tasks[i].scheduler.slack_time - tasks[i].scheduler.remaining_exec_time;
				if(br_send(payload, tasks[i].id, tasks[i].observer_address, MON_QOS))
					last_rt[i] = now;
			}
		}
	}
}
