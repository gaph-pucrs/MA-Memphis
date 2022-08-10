/**
 * MA-Memphis
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

#include <stdint.h>
#include <stdlib.h>

#include <memphis.h>

#include "interrupts.h"
#include "services.h"
#include "task_migration.h"
#include "pending_service.h"
#include "dmni.h"
#include "task_location.h"
#include "syscall.h"
#include "llm.h"
#include "stdio.h"
#include "monitor.h"
#include "string.h"
#include "pending_msg.h"

tcb_t *os_isr(unsigned int status)
{
	MMR_SCHEDULING_REPORT = REPORT_INTERRUPTION;

	if(sched_is_idle())
		sched_update_slack_time();

	bool call_scheduler = false;
	/* Check interrupt source */
	if(status & IRQ_BRNOC){
		br_packet_t br_packet;
		br_read(&br_packet);

		if(
			MMR_DMNI_SEND_ACTIVE && 
			(
				br_packet.service == MESSAGE_REQUEST || 
				br_packet.service == TASK_MIGRATION
			)
		){
			/* Fake a packet as a pending service */
			packet_t packet;
			br_fake_packet(&br_packet, &packet);
			// puts("Faking packet as pending service\n");
			pending_svc_push(&packet);
		} else {
			call_scheduler |= os_handle_broadcast(&br_packet);
		}
	} else if(status & IRQ_NOC){
		packet_t packet;
		dmni_read(&packet, PKT_SIZE);

		if(
			MMR_DMNI_SEND_ACTIVE && 
			(packet.service == DATA_AV || packet.service == MESSAGE_REQUEST)
		)
			pending_svc_push(&packet);
		else
			call_scheduler = os_handle_pkt(&packet);
		
	} else if(status & IRQ_PENDING_SERVICE){
		/* Pending packet. Handle it */

		packet_t *packet = pending_svc_pop();
		if(packet)
			call_scheduler = os_handle_pkt(packet);
		
	} else {
		if(status & IRQ_SLACK_TIME){
			/* Send a monitoring packet */

			/** @todo Send to whom? */
			// sched_report_slack_time();
			MMR_SLACK_TIME_MONITOR = 0;
		}

		/* Only check stack if scheduling interrupt and no message treated in this interrupt */
		if(
			(status & IRQ_SCHEDULER) &&
			!sched_is_idle() &&
			sched_check_stack()
		){
			tcb_t *current = sched_get_current();
			printf(
				"Task id %d aborted due to stack overflow\n", 
				tcb_get_id(current)
			);

			tcb_abort_task(current);
		}
	}

	call_scheduler |= status & IRQ_SCHEDULER;

	if(call_scheduler)
		sched_run();
	
	if(sched_is_idle()){
		sched_update_idle_time();

		MMR_SCHEDULING_REPORT = REPORT_IDLE;
	} else {
		MMR_SCHEDULING_REPORT = sched_get_current_id();
	}

    /* Runs the scheduled task */
    return sched_get_current();
}

bool os_handle_broadcast(br_packet_t *packet)
{
	// printf("Broadcast received %x\n", packet->service);
	int16_t addr_field = packet->payload >> 16;
	int16_t task_field = packet->payload;

	switch(packet->service){
		case CLEAR_MON_TABLE:
			/* Write to DMNI register the ID value */
			return os_clear_mon_table(task_field);
		case ANNOUNCE_MONITOR:
			return os_announce_mon(task_field, addr_field);
		case RELEASE_PERIPHERAL:
			return os_release_peripheral();
		case UPDATE_TASK_LOCATION:
			puts(
				"DEPRECATED: UPDATE_TASK_LOCATION is now embedded in DATA_AV/MESSAGE_REQUEST"
			);
			return false;
		case TASK_MIGRATION:
			return os_task_migration(task_field, addr_field);
		case DATA_AV:
			// printf("Received DATA_AV via BrNoC with pre-cons %x and pre-prod %x\n", task_field, packet->src_id);
			return os_data_available(
				br_convert_id(task_field, MMR_NI_CONFIG), 
				br_convert_id(packet->src_id, addr_field), 
				addr_field
			);
		case MESSAGE_REQUEST:
			return os_message_request(
				br_convert_id(packet->src_id, addr_field), 
				addr_field, 
				br_convert_id(task_field, MMR_NI_CONFIG)
			);
		case ABORT_TASK:
			return os_abort_task(task_field);
		default:
			printf(
				"ERROR: unknown broadcast %x at time %d\n", 
				packet->service, 
				MMR_TICK_COUNTER
			);
			return false;
	}
}

bool os_handle_pkt(volatile packet_t *packet)
{
	// printf("Packet received %x\n", packet->service);
	switch(packet->service){
		case MESSAGE_REQUEST:
			return os_message_request(
				packet->consumer_task, 
				packet->requesting_processor, 
				packet->producer_task
			);
		case MESSAGE_DELIVERY:
			// putsv("Packet length is ", packet->msg_length);
			return os_message_delivery(
				packet->consumer_task, 
				packet->producer_task, 
				packet->insert_request, 
				packet->msg_length
			);
		case DATA_AV:
			return os_data_available(
				packet->consumer_task, 
				packet->producer_task, 
				packet->requesting_processor
			);
		case TASK_ALLOCATION:
			/* Injector -> Kernel. No need to insert inside delivery */
			return os_task_allocation(
				packet->task_ID, 
				packet->code_size, 
				packet->data_size, 
				packet->bss_size, 
				packet->program_counter, 
				packet->mapper_task, 
				packet->mapper_address
			);
		case TASK_RELEASE:
			puts(
				"DEPRECATED: TASK_RELEASE should be inside MESSAGE_DELIVERY"
			);
			return false;
		case UPDATE_TASK_LOCATION:
			puts(
				"DEPRECATED: UPDATE_TASK_LOCATION is now embedded in DATA_AV/MESSAGE_REQUEST"
			);
			return false;
		case TASK_MIGRATION:
			return os_task_migration(
				packet->task_ID, 
				packet->allocated_processor
			);
		case MIGRATION_CODE:
			return os_migration_code(
				packet->task_ID, 
				packet->code_size, 
				packet->mapper_task, 
				packet->mapper_address
			);
		case MIGRATION_TCB:
			return os_migration_tcb(
				packet->task_ID, 
				packet->program_counter, 
				packet->period, 
				packet->deadline, 
				packet->execution_time, 
				packet->waiting_msg
			);
		case MIGRATION_TASK_LOCATION:
			return os_migration_tl(
				packet->task_ID, 
				packet->request_size, 
				packet->source_PE
			);
		case MIGRATION_MSG_REQUEST:
			return os_migration_mr(packet->task_ID, packet->request_size);
		case MIGRATION_DATA_AV:
			return os_migration_data_av(packet->task_ID, packet->request_size);
		case MIGRATION_PIPE:
			return os_migration_pipe(
				packet->task_ID, 
				packet->consumer_task, 
				packet->msg_length
			);
		case MIGRATION_STACK:
			return os_migration_stack(
				packet->task_ID, 
				packet->stack_size
			);
		case MIGRATION_HEAP:
			return os_migration_heap(packet->task_ID, packet->heap_size);
		case MIGRATION_DATA_BSS:
			return os_migration_data_bss(
				packet->task_ID, 
				packet->data_size, 
				packet->bss_size
			);
		default:
			printf("ERROR: unknown interrupt at time %d\n", MMR_TICK_COUNTER);
			return false;
	}
}

bool os_message_request(int cons_task, int cons_addr, int prod_task)
{
	bool force_sched = false;

	if(prod_task & MEMPHIS_KERNEL_MSG){
		/* Message directed to kernel */
		/* ATTENTION: Never request directly to kernel. Always use SReceive! */

		/* Search for the kernel-produced message */
		opipe_t *opipe = pend_msg_find(cons_task);
		
		if(opipe == NULL){
			puts(
				"ERROR: Kernel received request but message not found. Use memphis_receive_any!"
			);
			while(1);
		}

		/* Send it like a MESSAGE_DELIVERY */
		opipe_send(opipe, prod_task, cons_addr);
		pend_msg_remove(opipe);

		/* If still pending messages to requesting task, also send a data available */
		if(pend_msg_find(cons_task) != NULL){
			/* Send data available to the right processor */
			data_av_send(
				cons_task, 
				MEMPHIS_KERNEL_MSG | MMR_NI_CONFIG, 
				cons_addr, 
				MMR_NI_CONFIG
			);
		}
	} else {
		// printf("Received message request from task %d to task %d\n", cons_task, prod_task);

		/* Get the producer task */
		tcb_t *prod_tcb = tcb_search(prod_task);

		if(!prod_tcb){
			// puts("Producer NOT found. Will resend the request and update location\n");
			/* Task is not here. Probably migrated. */
			int migrated_addr = tm_get_migrated_addr(prod_task);
			// printf("Migrated address is %d\n", migrated_addr);

			/* Forward the message request to the migrated processor */
			mr_send(prod_task, cons_task, migrated_addr, cons_addr);

		} else {
			// puts("Producer found!\n");
			
			/* Update task location in case of migration */			
			if(
				!(cons_task & 0xFFFF0000) && 
				((cons_task >> 8) == (prod_task >> 8))
			){
				/* Only update if message came from another task of the same app */
				tl_insert_update(prod_tcb, cons_task, cons_addr);
			}
			
			/* Task found. Now search for message. */
			opipe_t *opipe = tcb_get_opipe(prod_tcb);
		
			if(opipe == NULL || opipe_get_cons_task(opipe) != cons_task){	/* No message in producer's pipe to the consumer task */
				/* Insert the message request in the producer's TCB */
				// puts("Message not found. Inserting message request.\n");
				mr_insert(prod_tcb, cons_task, cons_addr);
			} else {	/* Message found */
				if(cons_addr == MMR_NI_CONFIG){
					/* Message Request came from NoC but the producer migrated to this address */
					/* Writes to the consumer page address */
					tcb_t *cons_tcb = tcb_search(cons_task);
					
					if(!cons_tcb){
						puts("ERROR: CONS TCB NOT FOUND ON MR");
						while(true);
					}

					ipipe_t *ipipe = tcb_get_ipipe(cons_tcb);

					size_t buf_size;
					void *src = opipe_get_buf(opipe, &buf_size);

					int result = ipipe_transfer(
						ipipe, 
						tcb_get_offset(cons_tcb), 
						src, 
						buf_size
					);

					if(result != buf_size){
						puts("ERROR: could not transfer pipe on request");
						/**
						 * @todo while(1)?
						 */
						return false;
					}

					opipe_pop(opipe);
					tcb_destroy_opipe(prod_tcb);

					/* Release consumer task */
					sched_release_wait(cons_tcb);

					if(tcb_need_migration(cons_tcb)){
						tm_migrate(cons_tcb);
						force_sched = true;
					}
				} else {
					/* Send through NoC */
					// puts("Message found. Sending through NoC.\n");
					opipe_send(opipe, prod_task, cons_addr);
					tcb_destroy_opipe(prod_tcb);
				}

				/* Release task for execution if it was blocking another send */
				if(sched_is_waiting_request(prod_tcb)){
					sched_release_wait(prod_tcb);
					force_sched |= sched_is_idle();
					if(tcb_has_called_exit(prod_tcb)){
						tcb_terminate(prod_tcb);
					}
				}
			}
		}
	}
	
	return force_sched;
}

bool os_message_delivery(int cons_task, int prod_task, int prod_addr, size_t size)
{
	if(cons_task & MEMPHIS_KERNEL_MSG){
		/* This message was directed to kernel */
		size_t align_size = (size + 3) & ~3;
		void *rcvmsg = malloc(align_size);
		dmni_read(rcvmsg, align_size >> 2);

		/* Process the message like a syscall triggered from another PE */
		int ret = os_kernel_syscall(rcvmsg, align_size >> 2);

		free(rcvmsg);
		rcvmsg = NULL;

		return ret;
	} else {
		/* Get consumer task */
		// printf("Received delivery to task %d with size %d\n", cons_task, size);
		tcb_t *cons_tcb = tcb_search(cons_task);

		if(!cons_tcb){
			puts("ERROR: CONS TCB NOT FOUND ON MD");
			/**
			 * @todo Create an exception and abort task?
			 */
			while(true);
		}

		/* Update task location in case of migration */			
		if(!(prod_task & 0xFFFF0000) && ((prod_task >> 8) == (cons_task >> 8))){
			/* Only update if message came from another task of the same app */
			tl_insert_update(cons_tcb, prod_task, prod_addr);
		}
		/* No need to check if task migrated here. Once REQUEST is emitted a task cannot migrate */

		ipipe_t *ipipe = tcb_get_ipipe(cons_tcb);

		if(ipipe == NULL){
			puts("ERROR: BUFFER NOT ALLOCATED FOR MD");
			/**
			 * @todo Create an exception and abort task?
			 */
			while(true);
		}
		// printf("Message at virtual address %p\n", ipipe->buf);

		int result = ipipe_receive(ipipe, tcb_get_offset(cons_tcb), size);
		if(result != size){
			puts("ERROR: buffer failure on message delivery");
			/** 
			 * @todo Create an exception and abort task?
			 */
			while(true);
		}
		// puts("Message read from DMNI");

		/* Release task to execute */
		sched_release_wait(cons_tcb);
		// puts("Consumer released");

		if(tcb_need_migration(cons_tcb)){
			tm_migrate(cons_tcb);
			return true;
		}

		return sched_is_idle();
	}
}

bool os_data_available(int cons_task, int prod_task, int prod_addr)
{
	// printf("DATA_AV from id %x addr %x to id %x\n", prod_task, prod_addr, cons_task);
	if(cons_task & 0x10000000){
		/* This message was directed to kernel */
		/* Kernel is always ready to receive. Send message request */
		mr_send(prod_task, cons_task, prod_addr, MMR_NI_CONFIG);
	} else {
		/* Insert the packet received */
		tcb_t *cons_tcb = tcb_search(cons_task);

		if(cons_tcb){	/* Ensure task is allocated here */
			/* Update task location in case of migration */			
			if(
				!(prod_task & 0xFFFF0000) && 
				((prod_task >> 8) == (cons_task >> 8))
			){
				/* Only update if message came from another task of the same app */
				tl_insert_update(cons_tcb, prod_task, prod_addr);
			}

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

			/* Forward the message request to the migrated processor */
			data_av_send(cons_task, prod_task, migrated_addr, prod_addr);
		}
	}
	return false;
}

bool os_task_allocation(
	int id, 
	unsigned length, 
	unsigned data_len, 
	unsigned bss_len, 
	unsigned entry_point, 
	int mapper_task, 
	int mapper_addr
)
{
	tcb_t *free_tcb = tcb_free_get();
	// printf("TCB address is %x\n", (unsigned)free_tcb);
	// printf("TCB offset is %x\n", free_tcb->offset);

	/* Initializes the TCB */
	tcb_alloc(
		free_tcb, 
		id, 
		length, 
		data_len, 
		bss_len, 
		entry_point, 
		mapper_task, 
		mapper_addr
	);

	/* Clear the DATA_AV fifo of the task */
	data_av_init(free_tcb);

	/* Clear the task location array of the task */
	tl_init(free_tcb);

	/* Clears the message request table */
	mr_init(free_tcb);

	printf(
		"Task id %d allocated at %d with entry point %x\n", 
		id, 
		MMR_TICK_COUNTER, 
		entry_point
	);

	/* Obtain the program code */
	dmni_read(free_tcb->offset, (length + data_len) >> 2);

	// printf("Code lenght: %x\n", length);
	// printf("Mapper task: %d\n", mapper_task);
	// printf("Mapper addr: %d\n", mapper_addr);

	if(mapper_task != -1){
		/* Sends task allocated to mapper */
		return tl_send_allocated(free_tcb);
	} else {
		/* Task came from Injector directly. Release immediately */
		sched_release(free_tcb);
		return sched_is_idle();
	}
}

bool os_task_release(int id, int task_number, int *task_location)
{
	/* Get task to release */
	tcb_t *task = tcb_search(id);

	printf("-> TASK RELEASE received to task %d\n", task->id);
	// putsv("-> Task count: ", task_number);

	/* Write task location */
	memcpy(task->task_location, task_location, task_number*sizeof(int));

	/* If the task is blocked, release it */
	if(sched_is_blocked(task))
		sched_release(task);

	return sched_is_idle();
}

bool os_task_migration(int id, int addr)
{	
	tcb_t *task = tcb_search(id);

	if(task && !tcb_has_called_exit(task)){
		if(!tcb_need_migration(task)){
			printf("Trying to migrate task %d to address %d\n", id, addr);
			tcb_set_migrate_addr(task, addr);

			tm_send_code(task);

			llm_clear_table(task);

			if(!sched_is_waiting_delivery(task)){
				tm_migrate(task);
				return true;
			}
		} else {
			printf(
				"ERROR: task %x proc_to_migrate already assigned to %x when tried to assign %x\n", 
				id, 
				task->proc_to_migrate, 
				addr
			);
			while(1);
		}
	} else {
		printf("Tried to migrate task %x but it already terminated\n", id);
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

	/* Clears the message request table */
	mr_init(free_tcb);

	/* Obtain the program code */
	dmni_read((unsigned int*)tcb_get_offset(free_tcb), code_sz >> 2);

	// printf("Received MIGRATION_CODE from task id %d with size %d\n", id, code_sz);

	return false;
}

bool os_migration_tcb(
	int id, 
	unsigned int pc, 
	unsigned int period, 
	int deadline, 
	unsigned int exec_time, 
	unsigned waiting_msg
)
{
	tcb_t *tcb = tcb_search(id);

	tcb_set_pc(tcb, pc);

	dmni_read(tcb->registers, HAL_MAX_REGISTERS);

	/* Check if task has real time parameters */
	if(period)
		sched_real_time_task(tcb, period, deadline, exec_time);
	else
		sched_set_remaining_time(tcb, SCHED_MAX_TIME_SLICE);

	sched_set_waiting_msg(tcb, waiting_msg);

	// printf("Received MIGRATION_TCB from task id %d\n", id);

	return false;
}

bool os_migration_tl(int id, unsigned int tl_len, int source)
{
	tcb_t *tcb = tcb_search(id);

	dmni_read((unsigned int*)tcb->task_location, tl_len);

	// printf("Received MIGRATION_TASK_LOCATION from task id %d with size %d\n", id, tl_len);

	sched_release(tcb);

	printf(
		"Task id %d allocated by task migration at time %d from processor %x\n", 
		tcb_get_id(tcb), 
		MMR_TICK_COUNTER, 
		source
	);

	tl_update_local(id, MMR_NI_CONFIG);

	int task_migrated[2] = {TASK_MIGRATED, tcb->id};
	os_kernel_writepipe(
		task_migrated, 
		2*sizeof(int), 
		tcb->mapper_task, 
		tcb->mapper_address
	);

	return true;
}

bool os_migration_mr(int id, unsigned int mr_len)
{
	tcb_t *tcb = tcb_search(id);

	dmni_read(
		(unsigned int*)tcb->message_request, 
		mr_len*sizeof(message_request_t)/sizeof(unsigned int)
	);

	// printf("Received MIGRATION_MESSAGE_REQUEST from task id %d with size %d\n", id, mr_len);

	return false;
}

bool os_migration_data_av(int id , unsigned int data_av_len)
{
	tcb_t *tcb = tcb_search(id);

	dmni_read(
		(unsigned int*)data_av_get_buffer_tail(tcb), 
		data_av_len*sizeof(data_av_t)/sizeof(unsigned int)
	);

	data_av_add_tail(tcb, data_av_len);

	// printf("Received MIGRATION_DATA_AV from task id %d with size %d\n", id, data_av_len);

	return false;
}

bool os_migration_pipe(int id, int cons_task, size_t size)
{
	tcb_t *tcb = tcb_search(id);

	opipe_t *opipe = tcb_create_opipe(tcb);

	int result = opipe_receive(opipe, size, cons_task);

	if(result != size){
		puts("ERROR: not enough memory for pipe migration");
		/**
		 * @todo Abort task
		 */
	}

	// printf("Received MIGRATION_PIPE from task id %d with size %d\n", id, msg_len);

	return false;
}

bool os_migration_stack(int id, unsigned int stack_len)
{
	// putsv("Id received ", id);
	tcb_t *tcb = tcb_search(id);

	dmni_read(
		(unsigned int*)(tcb_get_offset(tcb) + PKG_PAGE_SIZE - stack_len), 
		stack_len >> 2
	);

	// printf("Received MIGRATION_STACK from task id %d with size %d\n", id, stack_len);

	return false;
}

bool os_migration_heap(int id, unsigned int heap_len)
{
	// putsv("Id received ", id);
	tcb_t *tcb = tcb_search(id);

	unsigned heap_start = tcb_get_heap_end(tcb);

	dmni_read((unsigned int*)(tcb_get_offset(tcb) + heap_start), heap_len >> 2);
	tcb_set_brk(tcb, heap_start + heap_len);

	// printf("Received MIGRATION_STACK from task id %d with size %d\n", id, stack_len);

	return false;
}

bool os_migration_data_bss(int id, unsigned int data_len, unsigned int bss_len)
{
	tcb_t *tcb = tcb_search(id);
	
	tcb_set_data_length(tcb, data_len);
	tcb_set_bss_length(tcb, bss_len);

	tcb_set_brk(tcb, tcb_get_code_length(tcb) + data_len + bss_len);

	dmni_read(
		(unsigned int*)(tcb_get_offset(tcb) + tcb_get_code_length(tcb)), 
		(bss_len + data_len) >> 2
	);

	return false;
}

bool os_clear_mon_table(int task)
{
	MMR_DMNI_CLEAR_MONITOR = task;
	return false;
}

bool os_announce_mon(enum MONITOR_TYPE type, int addr)
{
	llm_set_observer(type, addr);
	return false;
}

bool os_abort_task(int id)
{
	tcb_t *task = tcb_search(id);
	if(task){
		printf("Task id %d aborted by application\n", id);

		/* Send TASK_ABORTED */
		tl_send_terminated(task);

		/* Clear task from monitor tables */
		llm_clear_table(task);

		MMR_TASK_TERMINATED = id;

		int mig_addr = tcb_get_migrate_addr(task);
		if(mig_addr != -1){
			/* Task is migrating. Inform the destination processor of this */
			tm_abort_task(id, mig_addr);
		}

		sched_clear(task);

		tcb_clear(task);

		return (sched_get_current() == task);
	} else {
		/* Task already terminated or migrated from here */
		int addr = tm_get_migrated_addr(id);
		if(addr != -1){
			tm_abort_task(id, addr);
		}
		return false;
	}
}
