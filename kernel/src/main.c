/**
 * 
 * @file main.c
 *
 * @author Angelo Elias Dalzotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date August 2020
 * 
 * @brief Initialization procedures of the kernel.
 */

#include "mmr.h"
#include "hal.h"
#include "pending_service.h"
#include "task_migration.h"
#include "stdio.h"
#include "llm.h"

int main()
{
    hal_disable_interrupts();

	printf("Initializing PE %x\n", MMR_NI_CONFIG);

	pkt_init();

	tcb_init();

	pending_svc_init();
	pending_msg_init();

	sched_init();

	tm_init();

	llm_init();

	MMR_IRQ_MASK = (
		MMR_IRQ_BRNOC |
		MMR_IRQ_SCHEDULER | 
		MMR_IRQ_NOC | 
		MMR_IRQ_PENDING_SERVICE | 
		MMR_IRQ_SLACK_TIME
	);
	
	hal_run_task((void*)sched_get_current());

	while(true);
	return 0;
}
