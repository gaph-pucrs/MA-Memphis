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
#include "services.h"

void llm_rt(tcb_t *tasks)
{
	static unsigned last_rt[PKG_MAX_LOCAL_TASKS];
	
	unsigned now = MMR_TICK_COUNTER;

	for(int i = 0; i < PKG_MAX_LOCAL_TASKS; i++){
		int id = tasks[i].id;

		if(id == -1 || (id >> 8) == 0 || tasks[i].observer_task == -1 || tasks[i].scheduler.deadline == -1 || tasks[i].proc_to_migrate != -1)
			continue; /* Don't send MA task status or non-RT tasks or non-existent tasks or tasks marked to migrate */

		if(now - last_rt[i] >= PKG_MONITOR_INTERVAL_QOS){
			int message[5] = {
				MONITOR, 
				id,
				tasks[i].scheduler.waiting_msg,
				tasks[i].scheduler.slack_time, 
				tasks[i].scheduler.remaining_exec_time
			};

			if(!MMR_DMNI_SEND_ACTIVE){
				/* Avoid flooding DMNI and hanging too much time in LLM */
				os_kernel_writepipe(tasks[i].observer_task, tasks[i].observer_address, 5, message);
				last_rt[i] = now;
			}
		}
	}
}
