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
#include "syscall.h"
#include "llm.h"

void os_isr(unsigned int status)
{
	HAL_SCHEDULING_REPORT = HAL_INTERRUPTION;

	if(sched_is_idle())
		sched_update_slack_time();	

	bool call_scheduler = false;
	/* Check interrupt source */
	if(status & HAL_IRQ_NOC){
		volatile packet_t packet; 
		pkt_read(&packet);

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
		sched_run(); //chama o escalonador
	} else if(sched_is_idle()){
		sched_update_idle_time();

		HAL_SCHEDULING_REPORT = HAL_IDLE;

	} else {
		HAL_SCHEDULING_REPORT = sched_get_current_id();

	}

    /* Runs the scheduled task */
    hal_run_task((void*)sched_get_current());
}

bool os_handle_pkt(volatile packet_t *packet)
{
	switch(packet->service){
		case MESSAGE_REQUEST:
			return os_message_request(packet->consumer_task, packet->requesting_processor, packet->producer_task);
		case MESSAGE_DELIVERY:
			// putsv("Packet length is ", packet->msg_lenght);
			return os_message_delivery(packet->consumer_task, packet->msg_lenght);
		case DATA_AV:
			return os_data_available(packet->consumer_task, packet->producer_task, packet->requesting_processor);
		case TASK_ALLOCATION:
			/* Injector -> Kernel. No need to insert inside delivery */
			return os_task_allocation(packet->task_ID, packet->code_size, packet->mapper_task, packet->mapper_address);
		case TASK_RELEASE:
			puts("DEPRECATED: TASK_RELEASE should be inside MESSAGE_DELIVERY\n");
			return false;
		case UPDATE_TASK_LOCATION:
			return os_update_task_location(packet->consumer_task, packet->task_ID, packet->allocated_processor);
		case TASK_MIGRATION:
			puts("DEPRECATED: TASK_MIGRATION should be inside MESSAGE_DELIVERY\n");
			return false;
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
			putsv("ERROR: service unknown at time ", HAL_TICK_COUNTER);
			return false;
	}
}

bool os_message_request(int cons_task, int cons_addr, int prod_task)
{
	if(prod_task & 0x10000000){
		/* Message directed to kernel */
		/* ATTENTION: Never request directly to kernel. Always use SReceive! */

		/* Search for the kernel-produced message */
		pending_msg_t *msg = pending_msg_search(cons_task);
		if(!msg){
			puts("ERROR: Kernel received request but message not found. Use SReceive!\n");
			while(1);
		}
		/* Send it like a MESSAGE_DELIVERY */
		pending_msg_send(msg, cons_addr);

		/* If still pending messages to requesting task, also send a data available */
		if(pending_msg_search(cons_task) != NULL){
			/* Send data available to the right processor */
			data_av_send(cons_task, 0x10000000 | HAL_NI_CONFIG, cons_addr, HAL_NI_CONFIG);
		}
	} else {
		// putsvsv("Received message request from task ", cons_task, " to task ", prod_task);
		/* Get the producer task */
		tcb_t *prod_tcb = tcb_search(prod_task);

		if(!prod_tcb){
			// puts("Producer NOT found. Will resend the request and update location\n");
			/* Task is not here. Probably migrated. */
			int migrated_addr = tm_get_migrated_addr(prod_task);
			// putsv("Migrated address is ", migrated_addr);

			/* Update the task location in the consumer */
			tl_send_update(cons_task, cons_addr, prod_task, migrated_addr);

			/* Forward the message request to the migrated processor */
			mr_send(prod_task, cons_task, migrated_addr, cons_addr);

		} else {
			// puts("Producer found!\n");
			/* Task found. Now search for message. */
			pipe_t *message = pipe_pop(prod_tcb, cons_task);
		
			if(!message){	/* No message in producer's pipe to the consumer task */
				/* Insert the message request in the producer's TCB */
				// puts("Message not found. Inserting message request.\n");
				mr_insert(prod_tcb, cons_task, cons_addr);
			} else {	/* Message found */
				/* Send through NoC */
				// puts("Message found. Sending through NoC.\n");
				pipe_send(prod_task, cons_task, cons_addr, message);

				/* Release task for execution if it was blocking another send */
				if(sched_is_waiting_request(prod_tcb)){
					sched_release_wait(prod_tcb);
					return sched_is_idle();
				}
			}
		}
	}
	
	return false;
}

bool os_message_delivery(int cons_task, unsigned int length)
{
	if(cons_task & 0x10000000){
		/* This message was directed to kernel */
		/* Receive the message in stack. Maybe this is a bad idea. */
		/** @todo Check this behavior for big messages */
		static int rcvmsg[MSG_SIZE];
		dmni_read(rcvmsg, length);

		// puts("-- Received message to kernel");
		// for(int i = 0; i < length; i++){
		// 	putsvsv("V", i, "=", rcvmsg[i]);
		// }

		/* Process the message like a syscall triggered from another PE */
		return os_kernel_syscall(rcvmsg, length);
	} else {
		/* Get consumer task */
		// putsv("Received delivery to task ", cons_task);
		tcb_t *cons_tcb = tcb_search(cons_task);
		// if(cons_tcb)
		// 	puts("Found TCB\n");
		// else
		// 	puts("TCB not found\n");

		/* Message is stored in task's page + argument 1 from syscall */
		message_t *message = tcb_get_message(cons_tcb);
		// putsv("Message at address ", (unsigned int)message);

		/* Assert message requested is the received size */
		message->length = length;
		// putsv("Message length = ", message->length);

		/* Obtain message from DMNI */
		dmni_read(message->msg, message->length);

		// puts("Message read from DMNI\n");

		/* Release task to execute */
		sched_release_wait(cons_tcb);
		// puts("Consumer released\n");

		if(tcb_need_migration(cons_tcb)){
			tm_migrate(cons_tcb);
			return true;
		}

		return sched_is_idle();
	}
}

bool os_data_available(int cons_task, int prod_task, int prod_addr)
{
	if(cons_task & 0x10000000){
		/* This message was directed to kernel */
		/* Kernel is always ready to receive. Send message request */
		mr_send(prod_task, cons_task, prod_addr, HAL_NI_CONFIG);
	} else {
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

	putsvsv("Task id: ", id, " allocated at ", HAL_TICK_COUNTER);

	/* Obtain the program code */
	dmni_read((int*)free_tcb->offset, length);

	// putsv("Code lenght: ", length);
	// putsv("Mapper task: ", mapper_task);
	// putsv("Mapper addr: ", mapper_addr);

	if(mapper_task != -1){
		/* Sends task allocated to mapper */
		return tl_send_allocated(free_tcb);
	} else {
		/* Task came from Injector directly. Release immediately */
		sched_release(free_tcb);
		return sched_is_idle();
	}
	
}

bool os_task_release(
	int id, 
	int data_sz, 
	int bss_sz, 
	int observer_task, 
	int observer_address, 
	int task_number, 
	int *task_location
){
	/* Get task to release */
	tcb_t *task = tcb_search(id);

	putsv("-> TASK RELEASE received to task ", task->id);
	// putsv("-> Task count: ", task_number);

	/* Update TCB with received info */
	tcb_update_sections(task, data_sz, bss_sz);

	task->observer_task = observer_task;
	task->observer_address = observer_address;

	/* Write task location */
	/** @todo Use memcpy */
	for(int i = 0; i < task_number; i++){
		// putsv("TL ", task_location[i]);
		task->task_location[i] = task_location[i];
	}

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
	putsvsv("Trying to migrate task ", id, " to address ", addr);
	tcb_t *task = tcb_search(id);

	if(task && !tcb_need_migration(task)){
		tcb_set_migrate_addr(task, addr);

		tm_send_code(task);

		if(!sched_is_waiting_delivery(task)){
			tm_migrate(task);
			return true;
		}
	} else {
		puts ("ERROR: task not found or proc_to_migrate already assigned\n");
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
	dmni_read((int*)tcb_get_offset(free_tcb), code_sz);

	// putsvsv("Received MIGRATION_CODE from task id ", id, " with size ", code_sz);

	return false;
}

bool os_migration_tcb(int id, unsigned int pc, unsigned int period, int deadline, unsigned int exec_time)
{
	tcb_t *tcb = tcb_search(id);

	tcb_set_pc(tcb, pc);

	dmni_read((int*)tcb->registers, HAL_MAX_REGISTERS);

	/* Check if task has real time parameters */
	if(period)
		sched_real_time_task(tcb, period, deadline, exec_time);

	// putsv("Received MIGRATION_TCB from task id ", id);

	return false;
}

bool os_migration_tl(int id, unsigned int tl_len)
{
	tcb_t *tcb = tcb_search(id);

	dmni_read(tcb->task_location, tl_len);

	// putsvsv("Received MIGRATION_TASK_LOCATION from task id ", id, " with size ", tl_len);

	return false;
}

bool os_migration_mr(int id, unsigned int mr_len)
{
	tcb_t *tcb = tcb_search(id);

	dmni_read((int*)tcb->message_request, mr_len*sizeof(message_request_t)/sizeof(unsigned int));

	// putsvsv("Received MIGRATION_MESSAGE_REQUEST from task id ", id, " with size ", mr_len);

	return false;
}

bool os_migration_data_av(int id , unsigned int data_av_len)
{
	// puts("Received MIG_DATA_AV PKT\n");
	tcb_t *tcb = tcb_search(id);

	dmni_read((int*)data_av_get_buffer_tail(tcb), data_av_len*sizeof(data_av_t)/sizeof(unsigned int));

	data_av_add_tail(tcb, data_av_len);

	// putsvsv("Received MIGRATION_DATA_AV from task id ", id, " with size ", data_av_len);

	return false;
}

bool os_migration_pipe(int id, int cons_task, unsigned int msg_len)
{
	tcb_t *tcb = tcb_search(id);

	pipe_set_cons_task(tcb, cons_task);
	pipe_set_message_len(tcb, msg_len);
	dmni_read(tcb->pipe.message.msg, msg_len);

	// putsvsv("Received MIGRATION_PIPE from task id ", id, " with size ", msg_len);

	return false;
}

bool os_migration_stack(int id, unsigned int stack_len)
{
	// putsv("Id received ", id);
	tcb_t *tcb = tcb_search(id);

	dmni_read((int*)(tcb_get_offset(tcb) + PKG_PAGE_SIZE - stack_len*4), stack_len);

	// putsvsv("Received MIGRATION_STACK from task id ", id, " with size ", stack_len);

	return false;
}

bool os_migration_data_bss(int id, unsigned int data_len, unsigned int bss_len, int source)
{
	tcb_t *tcb = tcb_search(id);
	
	tcb->data_lenght = data_len;
	tcb->bss_lenght = bss_len;

	if(bss_len + data_len)
		dmni_read((int*)(tcb_get_offset(tcb) + tcb_get_code_length(tcb)*4), bss_len + data_len);

	sched_release(tcb);
	sched_set_remaining_time(tcb, SCHED_MAX_TIME_SLICE);

	puts("Task id: "); puts(itoa(tcb_get_id(tcb))); puts(" allocated by task migration at time "); puts(itoa(HAL_TICK_COUNTER)); puts(" from processor "); puts(itoh(source)); puts("\n");

	int task_migrated[2] = {TASK_MIGRATED, tcb->id};
	os_kernel_writepipe(tcb->mapper_task, tcb->mapper_address, 2, task_migrated);

	return true;
}
