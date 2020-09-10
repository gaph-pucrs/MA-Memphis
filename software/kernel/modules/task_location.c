/**
 * 
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

#include "task_location.h"
#include "services.h"
#include "pkg.h"
#include "packet.h"

void tl_init(tcb_t *tcb)
{
	for(int i = 0; i < PKG_MAX_TASKS_APP; i++)
		tcb->task_location[i] = -1;
}

void tl_send_update(int dest_task, int dest_addr, int updt_task, int updt_addr)
{
	packet_t *packet = pkt_slot_get();

	packet->header = dest_addr;
	packet->service = UPDATE_TASK_LOCATION;
	packet->consumer_task = dest_task;
	packet->task_ID = updt_task;
	packet->allocated_processor = updt_addr;

	pkt_send(packet, 0, 0);
}

void tl_send_allocated(tcb_t *allocated_task)
{
	packet_t *packet = pkt_slot_get();

	packet->header = allocated_task->mapper_address;
	packet->service = TASK_ALLOCATED;
	packet->task_ID = allocated_task->id;
	packet->mapper_task = allocated_task->mapper_task;

	pkt_send(packet, 0, 0);
}

void tl_insert_update(tcb_t *tcb, int id, int addr)
{
	tcb->task_location[id & 0x00FF] = addr;
}

void tl_send_terminated(tcb_t *tcb)
{
	packet_t *packet = pkt_slot_get();

	packet->header = tcb->mapper_address;
	packet->service = TASK_TERMINATED;
	packet->task_ID = tcb->id;
	packet->mapper_task = tcb->mapper_task;

	pkt_send(packet, 0, 0);
}

int tl_search(tcb_t *tcb, int task)
{
	return tcb->task_location[task & 0x00FF];
}

hal_word_t tl_get_len(tcb_t *tcb)
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
		if(tcbs[i].id & 0xFF00 == id & 0xFF00){
			/* Task of the same app. Update. */
			tcbs[i].task_location[id & 0x00FF] = addr;
		}
	}
}




/**Searches for the location of a given task
 * \param task_ID The ID of the task
 * \return The task location (processor address in XY)
 */
int get_task_location(int task_ID){

	for(int i=0; i<MAX_TASK_LOCATION; i++) {
		if (task_location[i].id == task_ID){
			return task_location[i].proc_address;
		}
	}
	return -1;
}

/**Remove a task_locaton instance
 * \param task_id Task ID of the instance to be removed
 * \return The location of the removed task, -1 if ERROR
 */
int remove_task_location(int task_id){

	int r_proc;

	for(int i=0; i<MAX_TASK_LOCATION; i++) {

		if (task_location[i].id == task_id){

			task_location[i].id = -1;

			r_proc = task_location[i].proc_address;

			task_location[i].proc_address = -1;
			//puts("Add task location - task id "); puts(itoa(task_ID)); puts(" proc "); puts(itoh(proc)); puts("\n");
			return r_proc;
		}
	}

	puts("ERROR - task not found to remove\n");
	while(1);
	return -1;
}

/**Clear/remove all task of the same application
 * \param app_ID Application ID
 */
void clear_app_tasks_locations(int app_ID){

	for(int i=0;i<MAX_TASK_LOCATION; i++){
		if (task_location[i].id != -1 && (task_location[i].id >> 8) == app_ID){
			//puts("Remove task location - task id "); puts(itoa(task_location[i].id)); puts(" proc "); puts(itoh(task_location[i].proc_address)); puts("\n");
			task_location[i].id = -1;
			task_location[i].proc_address = -1;
		}
	}
}
