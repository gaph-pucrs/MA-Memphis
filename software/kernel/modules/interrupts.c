/**
 * 
 * @file interrupts.c
 *
 * @author Angelo Elias Dalzotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date August 2020
 * 
 * @brief Defines the interrupts procedures of the kernel.
 */

#include "task_control.h"
#include "services.h"
#include "interrupts.h"
#include "dmni.h"
#include "task_migration.h"

void os_isr(hal_word_t status)
{
	*HAL_SCHEDULING_REPORT = HAL_INTERRUPTION;

	if(sched_is_idle())
		sched_update_slack_time();	

	bool call_scheduler = false;
	/* Check interrupt source */
	if(status & HAL_IRQ_NOC){
		packet_t packet = pkt_read();

		if(
			*HAL_DMNI_SEND_ACTIVE && (
				packet.service == MESSAGE_REQUEST || 
				packet.service == TASK_MIGRATION
			)
		){
			pending_svc_push(&packet);
		} else {
			call_scheduler = os_handle_pkt(&packet);
		}

	} else if(status & HAL_IRQ_PENDING_SERVICE){
		/* Pending packet. Handle it */

		packet_t *packet = pending_svc_pop();
		if(packet)
			call_scheduler = handle_packet(packet);
		
	} else if(status & HAL_IRQ_SLACK_TIME){
		/* Send a monitoring packet */

		// sched_report_slack_time();
		*HAL_SLACK_TIME_MONITOR = 0;
	}

	if(status & HAL_IRQ_SCHEDULER)
		call_scheduler = true;

	if(call_scheduler){
		sched_run();

	} else if(sched_is_idle()){
		sched_update_idle_time();

		*HAL_SCHEDULING_REPORT = HAL_IDLE;

	} else {
		*HAL_SCHEDULING_REPORT = sched_get_current_id();

	}

    /* Runs the scheduled task */
    hal_run_task(sched_get_current());
}

bool os_handle_pkt(packet_t *packet)
{
	switch(packet->service){
		case MESSAGE_REQUEST:
			return os_message_request(packet->consumer_task, packet->requesting_processor, packet->producer_task);
		case MESSAGE_DELIVERY:
			return os_message_delivery(packet->consumer_task, packet->msg_lenght);
		case DATA_AV:
			return os_data_available(packet->consumer_task, packet->producer_task, packet->requesting_processor);
		case TASK_ALLOCATION:
			return os_task_allocation(packet->task_ID, packet->code_size, packet->mapper_task, packet->mapper_address);
		case TASK_RELEASE:
			return os_task_release(packet->task_ID, packet->data_size, packet->bss_size, packet->app_task_number);
		case UPDATE_TASK_LOCATION:
			return os_update_task_location(packet->consumer_task, packet->task_ID, packet->allocated_processor);
	// #if MIGRATION_ENABLED
	// 	case TASK_MIGRATION:
	// 	case MIGRATION_CODE:
	// 	case MIGRATION_TCB:
	// 	case MIGRATION_TASK_LOCATION:
	// 	case MIGRATION_MSG_REQUEST:
	// 	case MIGRATION_STACK:
	// 	case MIGRATION_DATA_BSS:
	// 		need_scheduling = handle_migration(p, cluster_master_address);
	// 		break;
	// #endif
		default:
			putsv("ERROR: service unknown: ", *HAL_TICK_COUNTER);
			return false;
	}
}

bool os_message_request(int cons_task, int cons_addr, int prod_task)
{
	/* Get the producer task */
	tcb_t *prod_tcb = tcb_search(prod_task);

	if(!prod_tcb){
		/* Task is not here. Probably migrated. */
		int migrated_addr = tm_get_migrated_addr(cons_task);

		/* Update the task location in the consumer */
		tl_send_update(cons_task, cons_addr, prod_task, migrated_addr);

		/* Forward the message request to the migrated processor */
		mr_send(prod_task, cons_task, migrated_addr, cons_addr);

	} else {
		/* Task found. Now search for message. */
		pipe_t *message = pipe_pop(prod_tcb, cons_task);
	
		if(!message){	/* No message in producer's pipe to the consumer task */
			/* Insert the message request in the producer's TCB */
			mr_insert(prod_tcb, cons_task, cons_addr);

		} else {	/* Message found */
			/* Send through NoC */
			pipe_send(prod_task, cons_task, cons_addr, message);

			/* Release task for execution if it was blocking another send */
			if(sched_is_waiting_request(prod_tcb)){
				sched_release_wait(prod_tcb);
				return sched_is_idle();
			}
		}
	}

	return false;
}

bool os_message_delivery(int cons_task, hal_word_t length)
{
	/* Get consumer task */
	tcb_t *cons_tcb = tcb_search(cons_task);

	/* Message is stored in task's page + argument 1 from syscall */
	message_t *message = tcb_get_message(cons_tcb);

	/* Assert message requested is the received size */
	message->length = length;

	/* Obtain message from DMNI */
	dmni_read((hal_word_t)message->msg, message->length);

	/* Release task to execute */
	sched_release_wait(cons_tcb);

	if(tcb_need_migration(cons_tcb)){
		tm_migrate(cons_tcb);
		return true;
	}

	return sched_is_idle();
}

bool os_data_available(int cons_task, int prod_task, int prod_addr)
{
	/* Insert the packet received */
	tcb_t *cons_tcb = tcb_search(cons_task);

	if(cons_tcb){	/* Ensure task is allocated here */
		/* Insert the packet to TCB */
		data_av_insert(cons_tcb, prod_task, prod_addr);

		/* If the consumer task is waiting for a DATA_AV, release it */
		if(sched_is_waiting_data_av(cons_tcb)){
			sched_release_wait(cons_tcb);
			return sched_is_idle();
		}

	} else {
		/* Task migrated? Forward. */
		int migrated_addr = tm_get_migrated_addr(cons_task);

		/* Update the task location in the consumer */
		tl_send_update(prod_task, prod_addr, cons_task, migrated_addr);

		/* Forward the message request to the migrated processor */
		data_av_send(cons_task, prod_task, migrated_addr, prod_addr);
	}

	return false;
}

bool os_task_allocation(int id, int length, int mapper_task, int mapper_addr)
{
	tcb_t *free_tcb = tcb_free_get();

	/* Initializes the TCB */
	tcb_alloc(free_tcb, id, length, mapper_task, mapper_addr);

	/* Clear the DATA_AV fifo of the task */
	data_av_init(free_tcb);

	/* Clear the task location array of the task */
	tl_init(free_tcb);

	/* Initialize the pipe for the task */
	pipe_init(free_tcb);

	/* Clears the message request table */
	mr_init(free_tcb);

	puts("Task id: "); puts(itoa(id)); putsv(" allocated at ", *HAL_TICK_COUNTER);	

	/* Obtain the program code */
	dmni_read(free_tcb->offset, length);

	putsv("Code lenght: ", length);
	putsv("Mapper task: ", mapper_task);
	putsv("Mapper addr: ", mapper_addr);

	/* Sends task allocated to mapper */
	/** @todo This should be wrapped in a DATA_AV!!!! */
	tl_send_allocated(free_tcb);

	//printTaskInformations(tcb_ptr, 1, 1, 0);
	return sched_is_idle();
}

bool os_task_release(int id, int data_sz, int bss_sz, uint16_t task_number)
{
	/* Get task to release */
	tcb_t *task = tcb_search(id);

	//putsv("-> TASK RELEASE received to task ", p->task_ID);

	/* Update TCB with received info */
	tcb_update_sections(task, data_sz, bss_sz);

	/* Get the location of app's tasks */
	dmni_read((hal_word_t)task->task_location, task_number);

	/* If the task is blocked, release it */
	if(sched_is_blocked(task))
		sched_release(task);

	return sched_is_idle();
}

bool os_update_task_location(int dest_task, int updt_task, int updt_addr)
{
	/* Get target task */
	tcb_t *task = tcb_search(dest_task);

	tl_insert_update(task, updt_task, updt_addr);

	return false;
}
