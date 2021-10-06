/**
 * MA-Memphis
 * @file task_location.c
 *
 * @author Marcelo Ruaro (marcelo.ruaro@acad.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date June 2016
 * 
 * @brief Identify where other tasks are allocated.
 */

#include <stddef.h>

#include "task_location.h"
#include "services.h"
#include "packet.h"
#include "syscall.h"
#include "broadcast.h"

void tl_init(tcb_t *tcb)
{
	for(int i = 0; i < PKG_MAX_TASKS_APP; i++)
		tcb->task_location[i] = -1;
}

void tl_send_update(int dest_addr, int updt_task, int updt_addr, bool broadcast)
{
	uint8_t svc = broadcast ? BR_SVC_ALL : BR_SVC_TGT;
	
	uint32_t payload = 0;
	payload |= updt_addr << 16;
	payload |= updt_task & 0xFFFF;

	while(!br_send(payload, -1, dest_addr, UPDATE_TASK_LOCATION, svc));
}

bool tl_send_allocated(tcb_t *allocated_task)
{
	int task_allocated[2] = {TASK_ALLOCATED, allocated_task->id};
	return os_kernel_writepipe(allocated_task->mapper_task, allocated_task->mapper_address, 2, task_allocated);
}

void tl_insert_update(tcb_t *tcb, int id, int addr)
{
	tcb->task_location[id & 0x00FF] = addr;
}

bool tl_send_terminated(tcb_t *tcb)
{
	int task_terminated[2] = {TASK_TERMINATED, tcb->id};
	return os_kernel_writepipe(tcb->mapper_task, tcb->mapper_address, 2, task_terminated);
}

int tl_search(tcb_t *tcb, int task)
{
	return tcb->task_location[task & 0x00FF];
}

unsigned int tl_get_len(tcb_t *tcb)
{
	for(int i = 0; i < PKG_MAX_TASKS_APP; i++){
		if(tcb->task_location[i] == -1)
			return i;
	}
	return PKG_MAX_TASKS_APP;
}

int *tl_get_ptr(tcb_t *tcb)
{
	return tcb->task_location;
}

void tl_update_local(int id, int addr)
{
	tcb_t *tcbs = tcb_get();

	for(int i = 0; i < PKG_MAX_LOCAL_TASKS; i++){
		if((tcbs[i].id & 0xFF00) == (id & 0xFF00)){
			/* Task of the same app. Update. */
			tcbs[i].task_location[id & 0x00FF] = addr;
		}
	}
}
