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

#include "interrupts.h"
#include "services.h"
#include "task_migration.h"
#include "pending_service.h"
#include "dmni.h"
#include "task_location.h"
#include "utils.h"

void os_isr(unsigned int status)
{
	HAL_SCHEDULING_REPORT = HAL_INTERRUPTION;

	if(sched_is_idle())
		sched_update_slack_time();	

	bool call_scheduler = false;
	/* Check interrupt source */
	if(status & HAL_IRQ_NOC){
		packet_t packet = pkt_read();

		if(
			HAL_DMNI_SEND_ACTIVE && (
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
			call_scheduler = os_handle_pkt(packet);
		
	} else if(status & HAL_IRQ_SLACK_TIME){
		/* Send a monitoring packet */

		/** @todo Send to whom? */
		// sched_report_slack_time();
		HAL_SLACK_TIME_MONITOR = 0;
	}

	if(status & HAL_IRQ_SCHEDULER)
		call_scheduler = true;

	if(call_scheduler){
		sched_run();

	} else if(sched_is_idle()){
		sched_update_idle_time();

		HAL_SCHEDULING_REPORT = HAL_IDLE;

	} else {
		HAL_SCHEDULING_REPORT = sched_get_current_id();

	}

    /* Runs the scheduled task */
    hal_run_task((void*)sched_get_current());
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
		case TASK_MIGRATION:
			return os_task_migration(packet->task_ID, packet->allocated_processor);
		case MIGRATION_CODE:
			return os_migration_code(packet->task_ID, packet->code_size, packet->mapper_task, packet->mapper_address);
		case MIGRATION_TCB:
			return os_migration_tcb(packet->task_ID, packet->program_counter, packet->period, packet->deadline, packet->execution_time);
		case MIGRATION_TASK_LOCATION:
			return os_migration_tl(packet->task_ID, packet->request_size);
		case MIGRATION_MSG_REQUEST:
			return os_migration_mr(packet->task_ID, packet->request_size);
		case MIGRATION_DATA_AV:
			return os_migration_data_av(packet->task_ID, packet->request_size);
		case MIGRATION_PIPE:
			return os_migration_pipe(packet->task_ID, packet->consumer_task, packet->msg_lenght);
		case MIGRATION_STACK:
			return os_migration_stack(packet->task_ID, packet->stack_size);
		case MIGRATION_DATA_BSS:
			return os_migration_data_bss(packet->task_ID, packet->data_size, packet->bss_size, packet->source_PE);
		default:
			putsv("ERROR: service unknown: ", HAL_TICK_COUNTER);
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

bool os_message_delivery(int cons_task, unsigned int length)
{
	/* Get consumer task */
	tcb_t *cons_tcb = tcb_search(cons_task);

	/* Message is stored in task's page + argument 1 from syscall */
	message_t *message = tcb_get_message(cons_tcb);

	/* Assert message requested is the received size */
	message->length = length;

	/* Obtain message from DMNI */
	dmni_read(message->msg, message->length);

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

	puts("Task id: "); puts(itoa(id)); putsv(" allocated at ", HAL_TICK_COUNTER);	

	/* Obtain the program code */
	dmni_read((unsigned int*)free_tcb->offset, length);

	putsv("Code lenght: ", length);
	putsv("Mapper task: ", mapper_task);
	putsv("Mapper addr: ", mapper_addr);

	/* Sends task allocated to mapper */
	/** @todo This should be wrapped in a DATA_AV!!!! */
	tl_send_allocated(free_tcb);

	//printTaskInformations(tcb_ptr, 1, 1, 0);
	return sched_is_idle();
}

bool os_task_release(int id, int data_sz, int bss_sz, unsigned short task_number)
{
	/* Get task to release */
	tcb_t *task = tcb_search(id);

	//putsv("-> TASK RELEASE received to task ", p->task_ID);

	/* Update TCB with received info */
	tcb_update_sections(task, data_sz, bss_sz);

	/* Get the location of app's tasks */
	dmni_read((unsigned int*)task->task_location, task_number);

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

bool os_task_migration(int id, int addr)
{
	tcb_t *task = tcb_search(id);

	if(task && !tcb_need_migration(task)){
		tcb_set_migrate_addr(task, addr);

		tm_send_code(task);

		if(!sched_is_waiting_delivery(task)){
			tm_migrate(task);
			return true;
		}
	} else {
		puts ("ERROR: task not found or proc_to_migrated already assigned\n");
		while(1);
	}

	return false;
}

bool os_migration_code(int id, unsigned int code_sz, int mapper_task, int mapper_addr)
{
	tcb_t *free_tcb = tcb_free_get();

	/* Initializes the TCB */
	tcb_alloc_migrated(free_tcb, id, code_sz, mapper_task, mapper_addr);

	/* Clear the DATA_AV fifo of the task */
	data_av_init(free_tcb);

	/* Clear the task location array of the task */
	tl_init(free_tcb);

	/* Initialize the pipe for the task */
	pipe_init(free_tcb);

	/* Clears the message request table */
	mr_init(free_tcb);

	/* Obtain the program code */
	dmni_read((unsigned int*)tcb_get_offset(free_tcb), code_sz);

	return false;
}

bool os_migration_tcb(int id, unsigned int pc, unsigned int period, int deadline, unsigned int exec_time)
{
	tcb_t *tcb = tcb_search(id);
	/** @todo Check volatile for all messages read and written to/from dmni */

	tcb_set_pc(tcb, pc);

	dmni_read(tcb->registers, HAL_MAX_REGISTERS);
	tcb_set_sp(tcb, tcb_get_sp(tcb));

	/* Check if task has real time parameters */
	if(period)
		sched_real_time_task(tcb, period, deadline, exec_time);

	return false;
}

bool os_migration_tl(int id, unsigned int tl_len)
{
	tcb_t *tcb = tcb_search(id);

	dmni_read((unsigned int*)tcb->task_location, tl_len);

	return false;
}

bool os_migration_mr(int id, unsigned int mr_len)
{
	tcb_t *tcb = tcb_search(id);

	dmni_read((unsigned int*)tcb->message_request, mr_len*sizeof(message_request_t)/sizeof(unsigned int));

	return false;
}

bool os_migration_data_av(int id , unsigned int data_av_len)
{
	tcb_t *tcb = tcb_search(id);

	dmni_read((unsigned int*)data_av_get_buffer_tail(tcb), data_av_len*sizeof(data_av_t)/sizeof(unsigned int));

	data_av_add_tail(tcb, data_av_len);

	return false;
}

bool os_migration_pipe(int id, int cons_task, unsigned int msg_len)
{
	tcb_t *tcb = tcb_search(id);

	pipe_set_cons_task(tcb, cons_task);
	pipe_set_message_len(tcb, msg_len);
	dmni_read(tcb->pipe.message.msg, msg_len);

	return false;
}

bool os_migration_stack(int id, unsigned int stack_len)
{
	tcb_t *tcb = tcb_search(id);

	dmni_read((unsigned int*)(tcb_get_offset(tcb) + PKG_PAGE_SIZE - stack_len*4), stack_len);

	return false;
}

bool os_migration_data_bss(int id, unsigned int data_len, unsigned int bss_len, int source)
{
	tcb_t *tcb = tcb_search(id);
	
	tcb->data_lenght = data_len;
	tcb->bss_lenght = bss_len;

	/** @todo Review these sizes */
	if(bss_len + data_len)
		dmni_read((unsigned int*)(tcb_get_offset(tcb) + tcb_get_code_length(tcb)*4), bss_len + data_len);

	sched_release(tcb);
	sched_set_remaining_time(tcb, SCHED_MAX_TIME_SLICE);

	/** @todo Confirm migration to whatever triggered it */

	puts("Task id: "); puts(itoa(tcb_get_id(tcb))); puts(" allocated by task migration at time "); puts(itoa(HAL_TICK_COUNTER)); puts(" from processor "); puts(itoh(source)); puts("\n");

	return true;
}
