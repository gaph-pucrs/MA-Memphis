/**
 * 
 * @file task_control.c
 *
 * @author Marcelo Ruaro (marcelo.ruaro@acad.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date June 2016
 * 
 * @brief This module defines the task control block (TCB) functions.
 */

#include <stdbool.h>
#include <stddef.h>

#include "task_control.h"

tcb_t idle_tcb;						//!< TCB pointer used to run idle task
tcb_t tcbs[PKG_MAX_LOCAL_TASKS];	//!< TCB array

void tcb_idle_task()
{
	while(true)
		*HAL_CLOCK_HOLD = 1;
}

void tcb_init()
{
	idle_tcb.pc = (hal_word_t)&tcb_idle_task;
	idle_tcb.id = 0;
	idle_tcb.offset = 0;

	for(int i = 0; i < PKG_MAX_LOCAL_TASKS; i++){
		tcbs[i].id = -1;
		tcbs[i].pc = 0;
		tcbs[i].offset = PKG_PAGE_SIZE * (i + 1);
		tcbs[i].proc_to_migrate = -1;
	}
}

tcb_t *tcb_get()
{
	return tcbs;
}

tcb_t *tcb_get_idle()
{
	return &idle_tcb;
}

tcb_t *tcb_search(int task)
{
    for(int i = 0; i < PKG_MAX_LOCAL_TASKS; i++){
    	if(tcbs[i].id == task)
    		return &tcbs[i];
	}

    return NULL;
}

tcb_t* tcb_free_get()
{
    for(int i = 0; i < PKG_MAX_LOCAL_TASKS; i++){
		if(tcbs[i].scheduler.status == SCHED_FREE)
			return &tcbs[i];
	}

    puts("ERROR - no FREE TCB\n");
    while(1);
    return NULL;
}

void tcb_alloc(tcb_t *tcb, int id, hal_word_t code_sz, int mapper_task, int mapper_addr)
{
	tcb->pc = 0;
	tcb->id = id;
	tcb->text_lenght = code_sz;
	tcb->proc_to_migrate = -1;
	tcb->scheduler.remaining_exec_time = SCHED_MAX_TIME_SLICE;
	tcb->scheduler.status = SCHED_BLOCKED;
	tcb->mapper_address = mapper_addr;
	tcb->mapper_task = mapper_task;
}

void tcb_update_sections(tcb_t *tcb, hal_word_t data_sz, hal_word_t bss_sz)
{
	tcb->data_lenght = data_sz;
	tcb->bss_lenght = bss_sz;
	tcb->text_lenght -= data_sz;
}

message_t *tcb_get_message(tcb_t *tcb)
{
	return tcb_get_offset(tcb) | tcb->registers[HAL_REG_A1];
}

hal_word_t tcb_get_offset(tcb_t *tcb)
{
	return tcb->offset;
}

int tcb_get_appid(tcb_t *tcb)
{
	return tcb->id >> 8;
}
