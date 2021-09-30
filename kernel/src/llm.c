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

#include "services.h"

void llm_rt(tcb_t *tasks)
{
	// static unsigned last_sent = PKG_MAX_LOCAL_TASKS - 1;

	// unsigned i = (last_sent + 1) % PKG_MAX_LOCAL_TASKS;
	// while(i != last_sent){
	// 	if(tasks[i].id != -1 && tasks[i].observer_task != -1 && tasks[i].scheduler.deadline != -1 && !tasks[i].scheduler.waiting_msg){
	// 		int payload = tasks[i].scheduler.slack_time - tasks[i].scheduler.remaining_exec_time;
	// 		if(payload < 0){
	// 			// printf("DEADLINE MISS with value %X\n", payload);
	// 			if(br_send(payload, tasks[i].id, tasks[i].observer_address, MON_QOS)){
	// 				last_sent = i;
	// 				return;
	// 			}
	// 		}
	// 	}

	// 	i = (i + 1) % PKG_MAX_LOCAL_TASKS;
	// }

	// if(tasks[i].id != -1 && tasks[i].observer_task != -1 && tasks[i].scheduler.deadline != -1 && !tasks[i].scheduler.waiting_msg){
	// 	int payload = tasks[i].scheduler.slack_time - tasks[i].scheduler.remaining_exec_time;
	// 	if(payload < 0){
	// 		// printf("DEADLINE MISS with value %X\n", payload);
	// 		br_send(payload, tasks[i].id, tasks[i].observer_address, MON_QOS);
	// 	}
	// }

	static unsigned last_rt[PKG_MAX_LOCAL_TASKS];
	unsigned now = MMR_TICK_COUNTER;

	for(int i = 0; i < PKG_MAX_LOCAL_TASKS; i++){
		if(now - last_rt[i] >= 10000){
			if(tasks[i].id != -1 && tasks[i].observer_task != -1 && tasks[i].scheduler.deadline != -1 && !tasks[i].scheduler.waiting_msg){
				int payload = tasks[i].scheduler.slack_time - tasks[i].scheduler.remaining_exec_time;
				if(br_send(payload, tasks[i].id, tasks[i].observer_address, MON_QOS))
					last_rt[i] = now;
			}
		}
	}
}
