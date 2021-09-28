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

#include "services.h"

void llm_task(tcb_t *task)
{
	/* What parameters? Deadline? */
	/* Maybe send a full-featured status */
	/* Deadline, execution time, etc. LLM should not process too much info */

	/* Build a message */
	if(task->id != -1 && task->observer_task != -1 && task->scheduler.deadline != -1 && !task->scheduler.waiting_msg){
		int payload = task->scheduler.slack_time - task->scheduler.remaining_exec_time;
		
		while(!br_send(payload, task->id, task->observer_address, MON_QOS));
	}
}

void llm_rt(tcb_t *tasks)
{
	static unsigned last_rt = 0;
	unsigned now = MMR_TICK_COUNTER;
	if(now - last_rt > PKG_SLACK_TIME_WINDOW){
		last_rt = now;
		for(int i = 0; i < PKG_MAX_LOCAL_TASKS; i++){
			llm_task(&tasks[i]);
		}
	}
}
