/**
 * MA-Memphis
 * @file halc.c
 *
 * @author Angelo Elias Dalzotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date December 2021
 * 
 * @brief Hardware abstraction layer for RISC-V processors.
 */

#include <stddef.h>
#include <stdio.h>

#include "hal.h"
#include "task_control.h"
#include "task_scheduler.h"
#include "llm.h"
#include "task_location.h"

tcb_t *hal_exception_handler(unsigned cause, unsigned value, unsigned pc)
{
	switch(cause){
		case HAL_EXC_INST_ADDR_MISALG:
			printf("ERROR: Instruction address misaligned at virtual address %x\n", value);
			break;
		case HAL_EXC_INST_ACCS_FLT:
			printf("ERROR: Instruction access fault at virtual address %x\n", value);
			break;
		case HAL_EXC_ILLGL_INST:
			printf("ERROR: Illegal instruction %x at virtual address %x\n", value, pc);
			break;
		case HAL_EXC_BREAKPOINT:
			printf("ERROR: Breakpoint at virtual address %x\n", value);
			break;
		case HAL_EXC_LOAD_ADDR_MISALG:
			printf("ERROR: Load address misaligned at virtual address %x\n", value);
			break;
		case HAL_EXC_LOAD_ACCS_FLT:
			printf("ERROR: Load access fault at virtual address %x\n", value);
			break;
		case HAL_EXC_STORE_ADDR_MISALG:
			printf("ERROR: Store address misaligned at virtual address %x\n", value);
			break;
		case HAL_EXC_STORE_ACCS_FLT:
			printf("ERROR: Store access fault at virtual address %x\n", value);
			break;
		case HAL_EXC_ENV_CALL_FROM_S:
			puts("ERROR: Environment call from S-mode\n");
			break;
		case HAL_EXC_ENV_CALL_FROM_M:
			printf("ERROR: Environment call from M-mode from PC %x\n", pc);
			break;
		case HAL_EXC_INST_PAGE_FLT:
			printf("ERROR: Instruction page fault at virtual address %x\n", value);
			break;
		case HAL_EXC_LOAD_PAGE_FLT:
			printf("ERROR: Load page fault at virtual address %x\n", value);
			break;
		case HAL_EXC_STORE_PAGE_FLT:
			printf("ERROR: Store page fault at virtual address %x\n", value);
			break;
		default:
			printf("ERROR: Unknown exception %x\n", cause);
			break;
	}

	tcb_t *current = sched_get_current_tcb();
	if(current){
		printf("Task id %d aborted with cause %d\n", tcb_get_id(current), cause);
		tcb_abort_task(current);
		sched_run();
		return sched_get_current_tcb();
	}

	return current;
}
