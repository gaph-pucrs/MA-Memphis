/**
 * MA-Memphis
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
#include "interrupts.h"
#include "pending_msg.h"
#include "paging.h"
#include "application.h"

int main()
{
	printf("Initializing PE %x\n", MMR_NI_CONFIG);

	page_init();
	app_init();
	tcb_init();
	sched_init();
	pmsg_init();
	psvc_init();
	tm_init();
	llm_init();
	pkt_init();

	MMR_IRQ_MASK = (
		IRQ_BRNOC |
		IRQ_SCHEDULER | 
		IRQ_NOC | 
		IRQ_PENDING_SERVICE
	);
	
	return 0;
}
