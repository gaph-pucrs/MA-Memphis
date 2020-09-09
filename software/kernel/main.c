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

#include "hal.h"
#include "packet.h"
#include "task_control.h"
#include "pending_service.h"

int main()
{
    hal_disable_interrupts();

	puts("Initializing PE: "); puts(itoh(*HAL_NI_CONFIG)); puts("\n");

	pkt_init();

	tcb_init();

	pending_svc_init();

	sched_init();

	*HAL_IRQ_MASK = (
		HAL_IRQ_SCHEDULER | 
		HAL_IRQ_NOC | 
		HAL_IRQ_PENDING_SERVICE | 
		HAL_IRQ_SLACK_TIME
	);
	
	hal_run_task(sched_get_current());

	while(true);
}
