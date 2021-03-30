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

void llm_task(tcb_t *task)
{
	/* What parameters? Deadline? */
	/* Maybe send a full-featured status */
	/* Deadline, execution time, etc. LLM should not process too much info */

	/* Build a message */
	if(task->observer_task != -1){
		int message[4] = {MONITOR, task->scheduler.deadline, task->scheduler.period, task->scheduler.execution_time};
		os_kernel_writepipe(task->observer_task, task->observer_address, 4, message);
	}
}
