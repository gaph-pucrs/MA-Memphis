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

#include <stddef.h>

#include "task_control.h"
#include "utils.h"

tcb_t idle_tcb;						//!< TCB pointer used to run idle task
tcb_t tcbs[PKG_MAX_LOCAL_TASKS];	//!< TCB array

void tcb_idle_task()
{
	while(true)
		HAL_CLOCK_HOLD = 1;
}

void tcb_init()
{
	idle_tcb.pc = (unsigned int)&tcb_idle_task;
	idle_tcb.id = 0;
	idle_tcb.offset = 0;
	idle_tcb.proc_to_migrate = -1;
	idle_tcb.observer_task = -1;
	idle_tcb.observer_address = -1;

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

void tcb_alloc(tcb_t *tcb, int id, unsigned int code_sz, int mapper_task, int mapper_addr)
{
	tcb->pc = 0;
	tcb->id = id;
	tcb->text_lenght = code_sz;
	tcb->proc_to_migrate = -1;
	tcb->scheduler.remaining_exec_time = SCHED_MAX_TIME_SLICE;
	tcb->scheduler.status = SCHED_BLOCKED;
	tcb->mapper_address = mapper_addr;
	tcb->mapper_task = mapper_task;
	tcb->observer_address = -1;
	tcb->observer_task = -1;
}

void tcb_alloc_migrated(tcb_t *tcb, int id, unsigned int code_sz, int mapper_task, int mapper_addr)
{
	tcb->scheduler.status = SCHED_MIGRATING;
	tcb->id = id;
	tcb->text_lenght = code_sz;
	tcb->proc_to_migrate = -1;
	tcb->mapper_task = mapper_task;
	tcb->mapper_address = mapper_addr;
}

void tcb_update_sections(tcb_t *tcb, unsigned int data_sz, unsigned int bss_sz)
{
	tcb->data_lenght = data_sz;
	tcb->bss_lenght = bss_sz;
	tcb->text_lenght -= data_sz;
}

message_t *tcb_get_message(tcb_t *tcb)
{
	return (message_t*)(tcb_get_offset(tcb) | tcb->registers[HAL_REG_A1]);
}

unsigned int tcb_get_offset(tcb_t *tcb)
{
	return tcb->offset;
}

int tcb_get_appid(tcb_t *tcb)
{
	return tcb->id >> 8;
}

bool tcb_need_migration(tcb_t *tcb)
{
	return tcb->proc_to_migrate != -1;
}

int tcb_get_migrate_addr(tcb_t *tcb)
{
	return tcb->proc_to_migrate;
}

void tcb_set_migrate_addr(tcb_t *tcb, int addr)
{
	tcb->proc_to_migrate = addr;
}

unsigned int tcb_get_pc(tcb_t *tcb)
{
	return tcb->pc - tcb->offset;
}

unsigned int tcb_get_sp(tcb_t *tcb)
{
	return tcb->registers[HAL_REG_SP];
}

int tcb_get_id(tcb_t *tcb)
{
	return tcb->id;
}

unsigned int tcb_get_reg(tcb_t *tcb, int idx)
{
	return tcb->registers[idx];
}

message_request_t *tcb_get_mr(tcb_t *tcb)
{
	return tcb->message_request;
}

void tcb_clear(tcb_t *tcb)
{
	tcb->pc = 0;
	tcb->id = -1;
	tcb->proc_to_migrate = -1;
}

unsigned int tcb_get_code_length(tcb_t *tcb)
{
	return tcb->text_lenght;
}

unsigned int tcb_get_data_length(tcb_t *tcb)
{
	return tcb->data_lenght;
}

unsigned int tcb_get_bss_length(tcb_t *tcb)
{
	return tcb->bss_lenght;
}

void tcb_set_pc(tcb_t *tcb, unsigned int pc)
{
	tcb->pc = pc + tcb->offset;
}

void tcb_set_sp(tcb_t *tcb, unsigned int sp)
{
	tcb->registers[HAL_REG_SP] = sp + tcb->offset;
}
