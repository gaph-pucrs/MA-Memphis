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

#include "interrupts.h"

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include <memphis/services.h>

#include "syscall.h"
#include "pending_service.h"
#include "pending_msg.h"
#include "task_migration.h"
#include "dmni.h"
#include "mmr.h"
#include "llm.h"

tl_t *halter = NULL;

tcb_t *isr_isr(unsigned status)
{
	// puts("INT");
	sched_report_interruption();

	if(sched_is_idle())
		sched_update_slack_time();

	bool call_scheduler = false;
	/* Check interrupt source */
	if(status & IRQ_BRNOC){
		// puts("BR");
		bcast_t bcast_packet;
		bcast_read(&bcast_packet);

		if(
			MMR_DMNI_SEND_ACTIVE && 
			(
				bcast_packet.service == MESSAGE_REQUEST || 
				bcast_packet.service == TASK_MIGRATION
			)
		){
			/* Fake a packet as a pending service */
			packet_t *packet = malloc(sizeof(packet_t));
			if(packet == NULL){
				puts("FATAL: could not allocate memory.");
				while(1);
			}

			bcast_fake_packet(&bcast_packet, packet);
			// puts("Faking packet as pending service\n");
			psvc_push_back(packet);
		} else {
			call_scheduler |= isr_handle_broadcast(&bcast_packet);
		}
	} else if(status & IRQ_NOC){
		// puts("NOC");
		packet_t *packet = malloc(sizeof(packet_t));
		if(packet == NULL){
			puts("FATAL: could not allocate memory.");
			while(1);
		}

		dmni_read(packet, PKT_SIZE);

		if(
			MMR_DMNI_SEND_ACTIVE && 
			(packet->service == DATA_AV || packet->service == MESSAGE_REQUEST)
		){
			psvc_push_back(packet);
		} else {
			call_scheduler = isr_handle_pkt(packet);
			free(packet);
		}
		
	} else if(status & IRQ_PENDING_SERVICE){
		// puts("PEND");
		/* Pending packet. Handle it */

		packet_t *packet = psvc_front();

		if(packet == NULL){
			puts("FATAL: Pending interrupt but no packet.");
			while(1);
		}

		psvc_pop_front();
		call_scheduler = isr_handle_pkt(packet);
		free(packet);
		
	} else if(status & IRQ_SCHEDULER){
		// puts("SCHED");

		tcb_t *current = sched_get_current_tcb();

		if(current != NULL && tcb_check_stack(current)){
			printf(
				"Task id %d aborted due to stack overflow\n", 
				tcb_get_id(current)
			);

			tcb_abort_task(current);
		}
	}

	call_scheduler |= (status & IRQ_SCHEDULER);

	tcb_t *current;

	if(call_scheduler){
		sched_run();
		current = sched_get_current_tcb();
	} else {
		current = sched_get_current_tcb();
		if(current == NULL){
			sched_update_idle_time();
		} else {
			int id = tcb_get_id(current);
			sched_report(id);
		}
	}

    /* Runs the scheduled task */
    return current;
}

bool isr_handle_broadcast(bcast_t *packet)
{
	// printf("Broadcast received %x\n", packet->service);
	int16_t addr_field = packet->payload >> 16;
	int16_t task_field = packet->payload;

	switch(packet->service){
		case CLEAR_MON_TABLE:
			/* Write to DMNI register the ID value */
			return isr_clear_mon_table(task_field);
		case ANNOUNCE_MONITOR:
			return isr_announce_mon(task_field, addr_field);
		case RELEASE_PERIPHERAL:
			return sys_release_peripheral();
		case TASK_MIGRATION:
			return isr_task_migration(task_field, addr_field);
		case DATA_AV:
			// printf("Received DATA_AV via BrNoC with pre-cons %x and pre-prod %x\n", task_field, packet->src_id);
			return isr_data_available(
				bcast_convert_id(task_field, MMR_NI_CONFIG), 
				bcast_convert_id(packet->src_id, addr_field), 
				addr_field
			);
		case MESSAGE_REQUEST:
			return isr_message_request(
				bcast_convert_id(packet->src_id, addr_field), 
				addr_field, 
				bcast_convert_id(task_field, MMR_NI_CONFIG)
			);
		case ABORT_TASK:
			return isr_abort_task(task_field);
		case APP_TERMINATED:
			return isr_app_terminated(task_field);
		case HALT_PE:
			return isr_halt_pe(task_field, addr_field);
		default:
			printf(
				"ERROR: unknown broadcast %x at time %d\n", 
				packet->service, 
				MMR_TICK_COUNTER
			);
			return false;
	}
}

bool isr_handle_pkt(volatile packet_t *packet)//
{
	// printf("Packet received %x\n", packet->service);
	switch(packet->service){
		case MESSAGE_REQUEST:
			return isr_message_request(
				packet->consumer_task, 
				packet->requesting_processor, 
				packet->producer_task
			);
		case MESSAGE_DELIVERY:
			// putsv("Packet length is ", packet->msg_length);
			return isr_message_delivery(
				packet->consumer_task, 
				packet->producer_task, 
				packet->insert_request, 
				packet->msg_length,
				packet->payload_size
			);
		case DATA_AV:
			return isr_data_available(
				packet->consumer_task, 
				packet->producer_task, 
				packet->requesting_processor
			);
		case TASK_ALLOCATION:
			/* Injector -> Kernel. No need to insert inside delivery */
			return isr_task_allocation(
				packet->task_ID, 
				packet->code_size, 
				packet->data_size, 
				packet->bss_size, 
				(void*)packet->program_counter, 
				packet->mapper_task, 
				packet->mapper_address
			);
		case TASK_MIGRATION:
			return isr_task_migration(
				packet->task_ID, 
				packet->allocated_processor
			);
		case MIGRATION_CODE:
			return isr_migration_code(
				packet->task_ID, 
				packet->code_size, 
				packet->mapper_task, 
				packet->mapper_address
			);
		case MIGRATION_TCB:
			return isr_migration_tcb(
				packet->task_ID, 
				(void*)packet->program_counter,
				packet->waiting_msg
			);
		case MIGRATION_TASK_LOCATION:
			return isr_migration_app(
				packet->task_ID, 
				packet->request_size
			);
		case MIGRATION_SCHED:
			return isr_migration_sched(
				packet->task_ID, 
				packet->period, 
				packet->deadline, 
				packet->execution_time, 
				packet->waiting_msg,
				packet->source_PE
			);
		case MIGRATION_DATA_AV:
		case MIGRATION_MSG_REQUEST:
			return isr_migration_tl(
				packet->task_ID, 
				packet->request_size,
				packet->service
			);
		case MIGRATION_PIPE:
			return isr_migration_pipe(
				packet->task_ID, 
				packet->consumer_task, 
				packet->msg_length
			);
		case MIGRATION_STACK:
			return isr_migration_stack(
				packet->task_ID, 
				packet->stack_size
			);
		case MIGRATION_HEAP:
			return isr_migration_heap(packet->task_ID, packet->heap_size);
		case MIGRATION_DATA_BSS:
			return isr_migration_data_bss(
				packet->task_ID, 
				packet->data_size, 
				packet->bss_size
			);
		default:
			printf("ERROR: unknown interrupt at time %d\n", MMR_TICK_COUNTER);
			return false;
	}
}

bool isr_message_request(int cons_task, int cons_addr, int prod_task)
{
	bool force_sched = false;

	if(prod_task & MEMPHIS_KERNEL_MSG){
		/* Message directed to kernel */
		/* ATTENTION: Never request directly to kernel. Always use SReceive! */

		/* Search for the kernel-produced message */
		opipe_t *opipe = pmsg_find(cons_task);
		
		if(opipe == NULL){
			printf("FATAL: task %d @%x requested but %x has no message.\n", cons_task, cons_addr, prod_task);
			while(1);
		}

		/* Send it like a MESSAGE_DELIVERY */
		opipe_send(opipe, prod_task, cons_addr);

		pmsg_remove(opipe);

		/* If still pending messages to requesting task, also send a data available */
		if(pmsg_find(cons_task) != NULL){
			/* Send data available to the right processor */
			tl_t dav;
			tl_set(&dav, MEMPHIS_KERNEL_MSG | MMR_NI_CONFIG, MMR_NI_CONFIG);

			tl_send_dav(&dav, cons_task, cons_addr);
		}
	} else {
		// printf("Received message request from task %d to task %d\n", cons_task, prod_task);

		/* Get the producer task */
		tcb_t *prod_tcb = tcb_find(prod_task);

		if(prod_tcb == NULL){
			/* Task is not here. Probably migrated. */
			tl_t *mig = tm_find(prod_task);

			if(mig == NULL){
				puts("ERROR: Task migrated not found in db.");
				return false;
			}

			int migrated_addr = tl_get_addr(mig);
			// printf("Forwarding MR to address %d\n", migrated_addr);

			/* Forward the message request to the migrated processor */
			tl_t msgreq;
			tl_set(&msgreq, cons_task, cons_addr);
			tl_send_msgreq(&msgreq, prod_task, migrated_addr);
		} else {
			// puts("Producer found!\n");
			
			/* Update task location in case of migration */			
			if(
				((cons_task & 0xFFFF0000) == 0) && 
				((cons_task >> 8) == (prod_task >> 8))
			){
				/* Only update if message came from another task of the same app */
				app_t *app = tcb_get_app(prod_tcb);
				app_update(app, cons_task, cons_addr);
			}
			
			/* Task found. Now search for message. */
			opipe_t *opipe = tcb_get_opipe(prod_tcb);
		
			if(opipe == NULL || opipe_get_cons_task(opipe) != cons_task){	/* No message in producer's pipe to the consumer task */
				/* Insert the message request in the producer's TCB */
				// puts("Message not found. Inserting message request.\n");
				list_t *msgreqs = tcb_get_msgreqs(prod_tcb);
				tl_emplace_back(msgreqs, cons_task, cons_addr);
			} else {	/* Message found */
				if(cons_addr == MMR_NI_CONFIG){
					/* Message Request came from NoC but the consumer migrated to this address */
					/* Writes to the consumer page address */
					tcb_t *cons_tcb = tcb_find(cons_task);
					
					if(cons_tcb == NULL){
						puts("ERROR: CONS TCB NOT FOUND ON MR");
						while(true);
					}

					ipipe_t *ipipe = tcb_get_ipipe(cons_tcb);

					if(ipipe == NULL){
						puts("FATAL: ipipe not present");
						while(true);
					}

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
					sched_t *sched = tcb_get_sched(cons_tcb);
					sched_release_wait(sched);

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
				sched_t *sched = tcb_get_sched(prod_tcb);
				if(sched_is_waiting_msgreq(sched)){
					sched_release_wait(sched);
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

bool isr_message_delivery(int cons_task, int prod_task, int prod_addr, size_t size, unsigned pkt_payload_size)
{
	// puts("ISR Delivery");
	if(cons_task & MEMPHIS_KERNEL_MSG){
		/* This message was directed to kernel */
		size_t align_size = (size + 3) & ~3;
		void *rcvmsg = malloc(align_size);
		dmni_read(rcvmsg, align_size >> 2);

		/* Process the message like a syscall triggered from another PE */
		int ret = sys_kernel_syscall(rcvmsg, align_size >> 2);

		free(rcvmsg);

		return ret;
	} else {
		/* Get consumer task */
		// printf("Received delivery to task %d with size %d\n", cons_task, size);
		tcb_t *cons_tcb = tcb_find(cons_task);

		if(cons_tcb == NULL){
			puts("ERROR: CONS TCB NOT FOUND ON MD");
			/**
			 * @todo Create an exception and abort task?
			 */
			while(true);
		}

		/* Update task location in case of migration */			
		if(
			((cons_task & 0xFFFF0000) == 0) && 
			((cons_task >> 8) == (prod_task >> 8))
		){
			/* Only update if message came from another task of the same app */
			app_t *app = tcb_get_app(cons_tcb);
			app_update(app, prod_task, prod_addr);
		}
		/* No need to check if task migrated here. Once REQUEST is emitted a task cannot migrate */

		ipipe_t *ipipe = tcb_get_ipipe(cons_tcb);

		if(ipipe == NULL){
			puts("ERROR: BUFFER NOT ALLOCATED FOR MD");
			// printf("Packet payload size = %u\n", pkt_payload_size);
			unsigned flits_to_drop = (pkt_payload_size-11);
			dmni_drop_payload(flits_to_drop);
		}
		// printf("Message at virtual address %p\n", ipipe->buf);

		int result = ipipe_receive(ipipe, tcb_get_offset(cons_tcb), size);
		if(result != size){
			puts("ERROR: buffer failure on message delivery");
			// printf("Packet payload size = %u\n", pkt_payload_size);
			unsigned flits_to_drop = (pkt_payload_size-11);
			dmni_drop_payload(flits_to_drop);
		}
		// puts("Message read from DMNI");

		/* Release task to execute */
		sched_t *sched = tcb_get_sched(cons_tcb);
		sched_release_wait(sched);

		if(tcb_need_migration(cons_tcb)){
			tm_migrate(cons_tcb);
			return true;
		}

		return sched_is_idle();
	}
}

bool isr_data_available(int cons_task, int prod_task, int prod_addr)
{
	// printf("DATA_AV from id %x addr %x to id %x\n", prod_task, prod_addr, cons_task);
	if(cons_task & MEMPHIS_KERNEL_MSG){
		/* This message was directed to kernel */
		/* Kernel is always ready to receive. Send message request */
		tl_t msgreq;
		tl_set(&msgreq, cons_task, MMR_NI_CONFIG);

		tl_send_msgreq(&msgreq, prod_task, prod_addr);
	} else {
		/* Insert the packet received */
		tcb_t *cons_tcb = tcb_find(cons_task);

		if(cons_tcb != NULL){	/* Ensure task is allocated here */
			/* Update task location in case of migration */			
			if(
				((prod_task & 0xFFFF0000) == 0) && 
				((prod_task >> 8) == (cons_task >> 8))
			){
				/* Only update if message came from another task of the same app */
				app_t *app = tcb_get_app(cons_tcb);
				app_update(app, prod_task, prod_addr);
			}

			/* Insert the packet to TCB */
			list_t *davs = tcb_get_davs(cons_tcb);
			tl_t *dav = tl_emplace_back(davs, prod_task, prod_addr);
			if(dav == NULL){
				puts("FATAL");
				while(true);
			}

			// puts("INSERTED DATA_AV");

			/* If the consumer task is waiting for a DATA_AV, release it */
			sched_t *sched = tcb_get_sched(cons_tcb);
			if(sched_is_waiting_dav(sched)){
				// puts("RELEASING");
				sched_release_wait(sched);
				return sched_is_idle();
			}

		} else {
			/* Task migrated? Forward. */
			tl_t *mig = tm_find(cons_task);

			if(mig == NULL){
				puts("ERROR: Task migrated not found in db.");
				return false;
			}

			int migrated_addr = tl_get_addr(mig);

			/* Forward the message request to the migrated processor */
			tl_t dav;
			tl_set(&dav, prod_task, prod_addr);
			tl_send_dav(&dav, cons_task, migrated_addr);
		}
	}

	return false;
}

bool isr_task_allocation(
	int id, 
	size_t text_size, 
	size_t data_size, 
	size_t bss_size, 
	void *entry_point, 
	int mapper_task, 
	int mapper_addr
)
{
	tcb_t *tcb = malloc(sizeof(tcb_t));

	if(tcb == NULL){
		puts("FATAL: could not allocate TCB");
		while(true);
	}

	list_entry_t *entry = tcb_push_back(tcb);
	if(entry == NULL){
		puts("FATAL: could not allocate TCB");
		while(true);
	}

	/* Initializes the TCB */
	tcb_alloc(
		tcb, 
		id, 
		text_size, 
		data_size, 
		bss_size, 
		mapper_task, 
		mapper_addr,
		entry_point
	);

	printf(
		"Task id %d allocated at %d with entry point %p and offset %p\n", 
		id, 
		MMR_TICK_COUNTER, 
		entry_point,
		tcb_get_offset(tcb)
	);

	/* Obtain the program code */
	dmni_read(tcb_get_offset(tcb), (text_size + data_size) >> 2);

	// printf("Text size: %x\n", text_size);
	// printf("Mapper task: %d\n", mapper_task);
	// printf("Mapper addr: %d\n", mapper_addr);

	if(mapper_task != -1){
		/* Sends task allocated to mapper */
		return tcb_send_allocated(tcb);
	} else {
		/* Task came from Injector directly. Release immediately */
		sched_t *sched = tcb_get_sched(tcb);

		if(sched == NULL)
			sched = sched_emplace_back(tcb);

		if(sched == NULL){
			puts("FATAL: unable to allocate scheduler");
			while(true);
		}

		return sched_is_idle();
	}
}

bool isr_task_release(int id, int task_cnt, int *task_location)
{
	/* Get task to release */
	tcb_t *tcb = tcb_find(id);

	if(tcb == NULL){
		printf("Task not found to release\n");
		return false;
	}

	printf("-> TASK RELEASE received to task %d\n", tcb_get_id(tcb));
	// putsv("-> Task count: ", task_number);

	/* Write task location */
	app_t *app = tcb_get_app(tcb);

	size_t current_size = app_get_task_cnt(app);
	if(current_size != task_cnt){
		int result = app_copy_location(app, task_cnt, task_location);

		if(result < 0){
			puts("FATAL: Could not allocate memory for task location.");
			while(true);
		}
	}

	/* If the task is blocked, release it */
	sched_t *sched = tcb_get_sched(tcb);
	if(sched == NULL)
		sched = sched_emplace_back(tcb);
	
	if(sched == NULL){
		puts("FATAL: could not allocate scheduler");
		while(true);
	}

	return sched_is_idle();
}

bool isr_task_migration(int id, int addr)
{
	tcb_t *task = tcb_find(id);

	if(task == NULL || tcb_has_called_exit(task)){
		printf("Tried to migrate task %x but it already terminated\n", id);
		return false;
	}

	int old_addr = tcb_get_migrate_addr(task);
	if(old_addr != -1){
		printf(
			"ERROR: task %x PE already assigned to %x when tried to assign %x\n", 
			id, 
			old_addr, 
			addr
		);

		return false;
	}

	printf("Trying to migrate task %d to address %d\n", id, addr);

	llm_clear_table(id);

	tcb_set_migrate_addr(task, addr);

	/* Send constant .text section */
	tm_send_text(task, id, addr);

	sched_t *sched = tcb_get_sched(task);
	if(sched_is_waiting_delivery(sched))
		return false;

	tm_migrate(task);
	return true;
}

bool isr_migration_code(int id, size_t text_size, int mapper_task, int mapper_addr)
{
	tcb_t *tcb = malloc(sizeof(tcb_t));

	if(tcb == NULL){
		puts("FATAL: could not allocate TCB");
		while(true);
	}

	list_entry_t *entry = tcb_push_back(tcb);
	if(entry == NULL){
		puts("FATAL: could not allocate TCB");
		while(true);
	}

	/* Initializes the TCB */
	tcb_alloc(tcb, id, text_size, 0, 0, mapper_task, mapper_addr, 0);

	text_size = (text_size + 3) & ~3;

	/* Obtain the program code */
	void *offset = tcb_get_offset(tcb);
	dmni_read(offset, text_size >> 2);

	// printf("Received MIGRATION_CODE from task id %d with size %d to store at offset %p\n", id, text_size, tcb_get_offset(tcb));

	return false;
}

bool isr_migration_tcb(int id, void *pc, unsigned received)
{
	tcb_t *tcb = tcb_find(id);

	if(tcb == NULL){
		puts("FATAL: TCB not found");
		return false;
	}

	tcb_set_pc(tcb, pc);

	if(received != 0){
		ipipe_t *ipipe = tcb_create_ipipe(tcb);
		if(ipipe == NULL){
			puts("FATAL: not enough memory");
			return false;
		}
		ipipe_set_read(ipipe, received);
	}

	dmni_read(tcb_get_regs(tcb), HAL_MAX_REGISTERS);

	// printf("Received MIGRATION_TCB from task id %d\n", id);

	return false;
}

bool isr_migration_sched(int id, unsigned period, int deadline, unsigned exec_time, unsigned waiting_msg, int source)
{
	tcb_t *tcb = tcb_find(id);

	if(tcb == NULL){
		puts("FATAL: TCB not found");
		return false;
	}

	sched_t *sched = sched_emplace_back(tcb);

	if(sched == NULL){
		puts("FATAL: could not allocate sched");
		while (true);
	}

	if(period != 0)
		sched_real_time_task(sched, period, deadline, exec_time);

	sched_set_waiting_msg(sched, waiting_msg);

	printf(
		"Task id %d allocated by task migration at time %d from processor %x\n", 
		id, 
		MMR_TICK_COUNTER, 
		source
	);

	app_t *app = tcb_get_app(tcb);
	app_update(app, id, MMR_NI_CONFIG);

	tl_t *mapper = tcb_get_mapper(tcb);
	int task_migrated[] = {TASK_MIGRATED, id};
	sys_kernel_writepipe(
		task_migrated, 
		sizeof(task_migrated), 
		tl_get_task(mapper), 
		tl_get_addr(mapper)
	);

	return true;

}

bool isr_migration_tl(int id, size_t size, unsigned service)
{
	tcb_t *tcb = tcb_find(id);

	if(tcb == NULL){
		puts("FATAL: TCB not found");
		return false;
	}

	tl_t *vec = malloc(size*sizeof(tl_t));

	if(vec == NULL){
		puts("FATAL: could not allocate DAV/MR");
		while(true);
	}

	dmni_read(vec, (size*sizeof(tl_t)) >> 2);
	
	list_t *list = NULL;

	switch(service){
		case MIGRATION_DATA_AV:
			list = tcb_get_davs(tcb);
			break;
		case MIGRATION_MSG_REQUEST:
			list = tcb_get_msgreqs(tcb);
			break;
		default:
			break;
	}

	if(list == NULL){
		puts("ERROR: Invalid service");
		return false;
	}

	for(int i = 0; i < size; i++){
		if(list_push_back(list, &(vec[i])) == NULL){
			puts("ERROR: not enough memory");
			return false;
		}
	}

	// printf("Received tl %d from task id %d with size %d\n", service, id, size);

	return false;
}

bool isr_migration_pipe(int id, int cons_task, size_t size)
{
	tcb_t *tcb = tcb_find(id);

	if(tcb == NULL){
		puts("FATAL: TCB not found");
		return false;
	}

	opipe_t *opipe = tcb_create_opipe(tcb);

	if(opipe == NULL){
		puts("ERROR: not enough memory for opipe");
		return -ENOMEM;
	}

	int result = opipe_receive(opipe, size, cons_task);

	if(result != size){
		puts("FATAL: not enough memory for pipe migration");
		while(true);
	}

	// printf("Received MIGRATION_PIPE from task id %d with size %d\n", id, size);

	return false;
}

bool isr_migration_stack(int id, size_t size)
{
	// putsv("Id received ", id);
	tcb_t *tcb = tcb_find(id);

	if(tcb == NULL){
		puts("FATAL: TCB not found");
		return false;
	}

	dmni_read(
		tcb_get_offset(tcb) + MMR_PAGE_SIZE - size, 
		size >> 2
	);

	// printf("Received MIGRATION_STACK from task id %d with size %d\n", id, size);

	return false;
}

bool isr_migration_heap(int id, size_t heap_size)
{
	// putsv("Id received ", id);
	tcb_t *tcb = tcb_find(id);

	if(tcb == NULL){
		puts("FATAL: TCB not found");
		return false;
	}

	void *heap_start = tcb_get_heap_end(tcb);

	tcb_set_brk(tcb, heap_start + heap_size);

	/* Align */
	heap_size = (heap_size + 3) & ~3;

	dmni_read(
		tcb_get_offset(tcb) + (unsigned)heap_start, 
		heap_size >> 2
	);

	// printf("Received MIGRATION_HEAP from task id %d with size %d\n", id, heap_size);

	return false;
}

bool isr_migration_data_bss(int id, size_t data_size, size_t bss_size)
{
	tcb_t *tcb = tcb_find(id);

	if(tcb == NULL){
		puts("FATAL: TCB not found");
		return false;
	}
	
	tcb_set_data_size(tcb, data_size);
	tcb_set_bss_size(tcb, bss_size);

	size_t total_size = data_size + bss_size;

	tcb_set_brk(tcb, (void*)(tcb_get_text_size(tcb) + total_size));

	/* Align */
	total_size = (total_size + 3) & ~3;

	dmni_read(
		tcb_get_offset(tcb) + tcb_get_text_size(tcb), 
		total_size >> 2
	);

	// printf("Received MIGRATION_DATA_BSS from task id %d with size %d\n", id, total_size);

	return false;
}

bool isr_migration_app(int id, size_t task_cnt)
{
	app_t *app = app_find(id >> 8);

	if(app == NULL){
		puts("FATAL: App not found.");
		while(true);
	}

	int *tmploc = malloc(task_cnt*sizeof(int));
	if(tmploc == NULL){
		puts("FATAL: Could not allocate memory for task location.");
		while(true);
	}

	dmni_read(tmploc, task_cnt);

	size_t current_size = app_get_task_cnt(app);
	if(current_size == 0){
		/* App created but no location present. Obtain from migration */
		app_set_location(app, task_cnt, tmploc);
	} else {
		/* App already present with locations. Migration can be outdated */

		/* Dicard!!!! */
		free(tmploc);
		tmploc = NULL;
	}

	// printf("Received MIGRATION_APP from task id %d with size %d\n", id, task_cnt);

	return false;
}

bool isr_clear_mon_table(int task)
{
	MMR_DMNI_CLEAR_MONITOR = task;
	return false;
}

bool isr_announce_mon(enum MONITOR_TYPE type, int addr)
{
	llm_set_observer(type, addr);
	return false;
}

bool isr_abort_task(int id)
{
	tcb_t *tcb = tcb_find(id);
	if(tcb != NULL){
		printf("Task id %d aborted by application\n", id);

		int mig_addr = tcb_get_migrate_addr(tcb);
		if(mig_addr != -1){
			/* Task is migrating. Inform the destination processor of this */
			tm_abort_task(id, mig_addr);
		}

		tcb_remove(tcb);

		return (sched_get_current_tcb() == tcb);
	} else {
		/* Task already terminated or migrated from here */
		tl_t *tl = tm_find(id);
		int addr = tl_get_addr(tl);
		if(addr != -1){
			tm_abort_task(id, addr);
		}
		return false;
	}
}

bool isr_app_terminated(int id)
{
	tm_clear_app(id);

	if(halter == NULL)
		return false;

	return !sys_halt(halter);
}

bool isr_halt_pe(int task, int addr)
{
	/* Halt this PE */
	/* We can only halt when all resources are released */
	/* This allow us to check for memory leaks! */

	halter = malloc(sizeof(tl_t));
	tl_set(halter, task, addr);

	return !sys_halt(halter);
}
