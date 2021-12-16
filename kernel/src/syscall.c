/**
 * MA-Memphis
 * @file syscall.c
 *
 * @author Angelo Elias Dalzotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date August 2020
 * 
 * @brief Defines the syscall procedures of the kernel.
 */

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "syscall.h"
#include "services.h"
#include "memphis.h"
#include "task_location.h"
#include "task_migration.h"
#include "pending_service.h"
#include "interrupts.h"
#include "broadcast.h"
#include "llm.h"
#include "memphis.h"

bool schedule_after_syscall;	//!< Signals the HAL syscall to call scheduler
bool task_terminated;

int os_syscall(unsigned int service, unsigned int a1, unsigned int a2, unsigned int a3)
{
	// printf("Syscall called %d\n", service);
	schedule_after_syscall = false;
	task_terminated = false;
	int ret = 0;
	switch(service){
		case EXIT:
			ret = os_exit(a1);
			break;
		case WRITEPIPE:
			ret = os_writepipe(a1, a2, a3);
			break;
		case READPIPE:
			ret = os_readpipe(a1, a2, a3);
			break;
		case GETTICK:
			ret = os_get_tick();
			break;
		case REALTIME:
			ret = os_realtime(a1, a2, a3);
			break;
		case GETLOCATION:
			ret = os_get_location();
			break;
		case GETID:
			ret = os_get_id();
			break;
		case SCALL_PUTC:
			ret = os_putc(a1);
			break;
		case SCALL_PUTS:
			ret = os_puts((char*)a1);
			break;
		case SCALL_BR_SEND_ALL:
			ret = os_br_send_all(a1, a2);
			break;
		case SCALL_BR_SEND_TGT:
			ret = os_br_send_tgt(a1, a2, a3);
			break;
		case SCALL_MON_PTR:
			ret = os_mon_ptr((unsigned*)a1, a2);
			break;
		default:
			printf("ERROR: Unknown service %x\n", service);
			ret = 0;
			break;
	}

	if(schedule_after_syscall)
		sched_run();

	return ret;
}

bool os_exit(int status)
{
	schedule_after_syscall = true;

	tcb_t *current = sched_get_current();

	tcb_set_called_exit(current);

	if(pipe_is_full(current)){
		/* Don't erase task with message in pipe */
		sched_set_wait_request(current);
		schedule_after_syscall = true;
		return false;
	}

	/* Avoid terminating a task with a message being sent */
	if(MMR_DMNI_SEND_ACTIVE)
		return false;

	task_terminated = true;

	/* Send TASK_TERMINATED */
	tl_send_terminated(current);

	/* Clear task from monitor tables */
	llm_clear_table(current);

	printf("Task id %d terminated with status %d\n", current->id, status);
	MMR_TASK_TERMINATED = current->id;

	sched_clear(current);

	tcb_clear(current);

	return true;
}

bool os_writepipe(unsigned int msg_ptr, int cons_task, bool sync)
{
	tcb_t *current = sched_get_current();

	if((cons_task & 0xFFFF0000) && current->id >> 8 != 0){
		puts("ERROR: Unauthorized message to kernel/peripheral from task with app id > 0\n");
		while(1);
	}

	int cons_addr;
	if((cons_task & MEMPHIS_FORCE_PORT)){
		/* Message directed to peripheral */
		cons_task &= (MEMPHIS_FORCE_PORT | 0x60000000 | 0x0000FFFF);
		cons_addr = cons_task;
	} else if((cons_task & MEMPHIS_KERNEL_MSG)){
		/* Message directed to kernel */
		cons_task &= (MEMPHIS_KERNEL_MSG | 0x0000FFFF);
		cons_addr = cons_task & 0x0000FFFF;
	} else {
		/* Send to task of the same app */
		cons_task &= 0x000000FF;
		cons_addr = tl_search(current, cons_task);
		cons_task |= (current->id & 0x0000FF00);
		// printf("Trying to write pipe from %d to %d at %d\n", current->id, cons_task, cons_addr);

		/* Check if consumer task is allocated */
		if(cons_addr == -1){
			/* Task is blocked until a TASK_RELEASE packet is received */
			sched_block(current);
			schedule_after_syscall = 1;
			return false;
		}
	}

	/* Points the message in the task page. Address composition: offset + msg address */
	// printf("Message at virtual address %x\n", msg_ptr);
	message_t *message = (message_t*)(tcb_get_offset(current) | msg_ptr);
	// printf("Message at physical address %x\n", (int)message);
	// printf("Message length = %d\n", message->length);

	/* Test if the application passed a invalid message lenght */
	if(message->length > PKG_MAX_MSG_SIZE){
		printf("ERROR: Message length of %d must be lower than PKG_MAX_MSG_SIZE\n", message->length);
		while(1);
	}

	/* Searches if there is a message request to the produced message */
	message_request_t *request = mr_peek(current, cons_task);

	if(request){	/* Message request found! */
		// puts("MR Found!\n");
		if(request->requester_address == MMR_NI_CONFIG){ 
			/* Local consumer */
			if(cons_task & 0x10000000){
				/* Message directed to kernel. No TCB to write to */
				/* This should NEVER happen because it means the kernel made a request without receiving a DATA_AV */
				puts("ERROR: Kernel made a request without receiving DATA_AV!\n");
				while(1);
				return false;
			} else {
				/* Writes to the consumer page address */
				tcb_t *cons_tcb = tcb_search(cons_task);
				message_t *msg_dst = tcb_get_message(cons_tcb);

				if(!cons_tcb){
					puts("ERROR: CONS TCB NOT FOUND ON WRITEPIPE\n");
					while(true);
				}

				pipe_transfer(message, msg_dst);

				/* Remove the message request from buffer */
				mr_pop(request, current->id);

				/* Release consumer task */
				sched_release_wait(cons_tcb);

				if(tcb_need_migration(cons_tcb)){
					tm_migrate(cons_tcb);
					schedule_after_syscall = 1;
				}
			}
		} else {
			// puts("Remote consumer\n");
			/* Remote consumer */

			/* Send a MESSAGE_DELIVERY */

			/* Deadlock avoidance: avoid sending a packet when the DMNI is busy */
			/* Also, don't send a message if the previous is still in pipe */
			if(MMR_DMNI_SEND_ACTIVE || pipe_is_full(current))
				return false;

			/* Insert the message in the pipe to avoid overwrite by task */
			pipe_push(current, message, cons_task);
			// puts("Pushed pipe\n");

			/* Send through NoC */
			pipe_t *pipe = pipe_pop(current, cons_task);
			// putsv("Popped message length = ", pipe->message.length);
			
			pipe_send(sched_get_current_id(), cons_task, request->requester_address, pipe);
			// puts("Sent through NoC\n");
			
			/* Remove the message request from buffer */
			mr_pop(request, current->id);
			// puts("Request popped\n");
		}
	} else if(!pipe_is_full(current) && !MMR_DMNI_SEND_ACTIVE){	/* Pipe is free */
		// puts("MR NOT found!\n");
		if(sync){
			// puts("Sync!\n");
			int prod_task = sched_get_current_id();
			if(cons_addr == MMR_NI_CONFIG){
				/* Local consumer */
				if(cons_task & 0x10000000){
					/* Message directed to kernel. No TCB to write to */
					/* We can bypass the need to kernel answer if a request */
					schedule_after_syscall = os_kernel_syscall(message->payload, message->length);
					return true;
				} else {
					/* Insert a DATA_AV to consumer table */
					tcb_t *cons_tcb = tcb_search(cons_task);

					/* Insert DATA_AV to the consumer TCB */
					data_av_insert(cons_tcb, prod_task, MMR_NI_CONFIG);
					
					/* If consumer waiting for a DATA_AV, release the task */
					if(sched_is_waiting_data_av(cons_tcb))
						sched_release_wait(cons_tcb);
				}
			} else {
				/* Send DATA_AV to consumer PE */
				data_av_send(cons_task, prod_task, cons_addr, MMR_NI_CONFIG);
			}
		}

		/* Please use SSend to send Kernel/Peripheral messages or it can be stuck in pipe forever */
		/* Store message in Pipe. Will be sent when a REQUEST is received */
		pipe_push(current, message, cons_task);
		// puts("Pushed to pipe!\n");

	} else {
		/* DMNI busy sending message or pipe full */
		if(pipe_is_full(current)){
			// puts("Request not found and pipe is full. Will wait until consumed!\n");
			/* In this case, we must wait for a message request to release the pipe */
			sched_set_wait_request(current);
			schedule_after_syscall = 1;
		} /* else: the DMNI is sending the older message. Must retry */
		
		return false;
	}

	return true;
}

bool os_readpipe(unsigned int msg_ptr, int prod_task, bool sync)
{
	tcb_t *current = sched_get_current();
	int cons_task = sched_get_current_id();

	int prod_addr;
	if(!sync){	/* Not synced READ must define the producer */
		prod_task |= (cons_task & 0xFF00);
		prod_addr = tl_search(current, prod_task);
		// putsvsv("Trying to read from task ", prod_task, " at address ", prod_addr);

		/* Test if the producer task is not allocated */
		if(prod_addr == -1){
			/* Task is blocked until its a TASK_RELEASE packet */
			sched_block(current);

			return false;
		}

		// printf("Readpipe: trying to read from task %d with address %d\n", prod_task, prod_addr);
		
	} else {
		/* Synced READ, must see what applications have data available for it */
		data_av_t *data_av = data_av_peek(current);
		
		if(!data_av){	/* No data available for requesting */
			/* Block task and wait for DATA_AV packet */
			sched_set_wait_data_av(current);
			schedule_after_syscall = 1;

			return false;
		}

		prod_task = data_av->requester;
		prod_addr = data_av->requester_addr;
		// printf("Readpipe: received DATA_AV from task %x with address %x\n", prod_task, prod_addr);
	}

	if(prod_addr == MMR_NI_CONFIG){	/* Local producer */
		if(prod_task & 0x10000000){
			// puts("Received DATA_AV from LOCAL Kernel!\n");
			/* Message from Kernel. No request needed */
			/* Search for the kernel-produced message */
			pending_msg_t *msg = pending_msg_search(cons_task);
	
			/* Store it like a MESSAGE_DELIVERY */
			message_t *message = (message_t*)(tcb_get_offset(current) | msg_ptr);
			
			// putsv("Message length is ", msg->size);
			// putsv("First word is ", msg->message[0]);
			message->length = msg->size;

			memcpy(message->payload, msg->message, msg->size * sizeof(message->payload[0]));
			
			/* Free pending message */
			msg->task = -1;

			/* Remove pending DATA_AV */
			data_av_pop(current);

			/* Add a new data available if kernel has message to this task */
			if(pending_msg_search(cons_task) != NULL){
				/* Add a new to the last position of the FIFO */
				data_av_insert(current, 0x10000000 | MMR_NI_CONFIG, MMR_NI_CONFIG);
			}

			return true;
		} else {
			// puts("Local producer\n");
			/* Get the producer TCB */
			tcb_t *prod_tcb = tcb_search(prod_task);

			if(!prod_tcb){
				puts("ERROR: PROD TCB NOT FOUND ON READPIPE\n");
				while(true);
			}

			/* Searches if the message is in PIPE (local producer) */
			pipe_t *pipe = pipe_pop(prod_tcb, cons_task);

			if(!pipe){
				/* Stores the request into the message request table */
				mr_insert(prod_tcb, cons_task, MMR_NI_CONFIG);

			} else {
				/* Message was found in pipe, writes to the consumer page address (local producer) */
				message_t *message = (message_t*)(tcb_get_offset(current) | msg_ptr);
				message_t *msg_src = pipe_get_message(pipe);

				pipe_transfer(msg_src, message);

				if(sched_is_waiting_request(prod_tcb))
					sched_release_wait(prod_tcb);

				return true;
			}
		}
	} else { /* Remote producer : Sends the message request */
		// puts("Remote producer\n");
		/* Deadlock avoidance: avoids to send a packet when the DMNI is busy in send process */
		if(MMR_DMNI_SEND_ACTIVE)
			return false;
		
		if(sync){
			/* DATA_AV is processed, erase it */
			data_av_pop(current);
		}

		/* Send the message request through NoC */
		mr_send(prod_task, cons_task, prod_addr, MMR_NI_CONFIG);
		// puts("Sent request\n");
	}

	/* Sets task as waiting blocking its execution, it will execute again when the message is produced by a WRITEPIPE or incoming MSG_DELIVERY */
	sched_set_wait_delivery(current);
	schedule_after_syscall = 1;

	return true;
}

unsigned int os_get_tick()	
{	
	return MMR_TICK_COUNTER;	
}

bool os_realtime(unsigned int period, int deadline, unsigned int exec_time)
{
	tcb_t *current = sched_get_current();
	sched_real_time_task(current, period, deadline, exec_time);

	schedule_after_syscall = 1;

	return true;
}

bool os_kernel_syscall(unsigned int *message, int length)
{
	/* Process it like a syscall */
	switch(message[0]){
		case TASK_RELEASE:
			// putsv("will call release with ntasks=", message[4]);
			return os_task_release(
				message[1], 
				message[2], 
				(int*)&message[3]
			);
		case TASK_MIGRATION:
			puts("DEPRECATED: TASK_MIGRATION should be sent by BrNoC\n");
			return false;
		case RELEASE_PERIPHERAL:
			puts("DEPRECATED: RELEASE_PERIPHERAL should be sent by BrNoC\n");
			return false;
		default:
			printf("ERROR: Unknown service %x inside MESSAGE_DELIVERY", message[0]);
			return false;
	}
}

bool os_kernel_writepipe(int task, int addr, int size, int *msg)
{
	/* Send data available only if target task hasn't received data available from this source */
	bool send_data_av = (pending_msg_search(task) == NULL);

	/* Avoid overwriting pending messages */
	while(MMR_DMNI_SEND_ACTIVE);

	// printf("Kernel writing pending message to task %d with size %d\n", task, size);
	/* Insert message in kernel output message buffer */
	pending_msg_push(task, size, msg);

	if(send_data_av){
		/* Check if local consumer / migrated task */
		tcb_t *cons_tcb = NULL;
		if(addr == MMR_NI_CONFIG){
			cons_tcb = tcb_search(task);
			if(!cons_tcb)
				addr = tm_get_migrated_addr(task);
		}

		if(cons_tcb){
			/* Insert the packet to TCB */
			data_av_insert(cons_tcb, 0x10000000 | MMR_NI_CONFIG, MMR_NI_CONFIG);

			/* If the consumer task is waiting for a DATA_AV, release it */
			if(sched_is_waiting_data_av(cons_tcb)){
				sched_release_wait(cons_tcb);
				return sched_is_idle();
			}
		} else {
			/* Send data available to the right processor */
			data_av_send(task, 0x10000000 | MMR_NI_CONFIG, addr, MMR_NI_CONFIG);
		}
	}

	return false;
}

bool os_release_peripheral()
{
	MMR_MEM_REG_PERIPHERALS = 1;
	// puts("Peripherals released\n");
	return false;
}

int os_get_location()
{
	return MMR_NI_CONFIG;
}

int os_get_id()
{
	return sched_get_current()->id;
}

int os_putc(char c)
{
	MMR_UART_CHAR = c;

	return 0;
}

int os_puts(char *str)
{
	tcb_t *current = sched_get_current();
	int id = sched_get_current_id();
	int addr = MMR_NI_CONFIG;
	char *msg_ptr = (char*)(tcb_get_offset(current) | (unsigned int)str);
	printf("$$$_%dx%d_%d_%d_", addr >> 8, addr & 0xFF, id >> 8, id & 0xFF);
	puts(msg_ptr);
	puts("\n");

	return 0;
}

int os_br_send_all(uint32_t payload, uint8_t ksvc)
{
	int producer = sched_get_current_id();
	
	if(producer >> 8 != 0)	/* AppID should be 0 */
		return 0;

	br_packet_t packet;
	packet.service = ksvc;
	packet.src_id = producer;
	packet.payload = payload;

	if(!br_send(&packet, MMR_NI_CONFIG, BR_SVC_ALL))
		return false;
		
	packet.src_addr = MMR_NI_CONFIG;
	schedule_after_syscall = os_handle_broadcast(&packet);
	return true;
}

int os_br_send_tgt(uint32_t payload, uint16_t target, uint8_t ksvc)
{
	int producer = sched_get_current_id();
	
	if(producer >> 8 != 0)	/* AppID should be 0 */
		return 0;

	br_packet_t packet;
	packet.service = ksvc;
	packet.src_id = producer;
	packet.payload = payload;

	if(target == MMR_NI_CONFIG){
		packet.src_addr = MMR_NI_CONFIG;
		schedule_after_syscall = os_handle_broadcast(&packet);
		return true;
	}

	return br_send(&packet, target, BR_SVC_TGT);
}

int os_mon_ptr(unsigned* table, enum MONITOR_TYPE type)
{
	tcb_t *current = sched_get_current();
	
	if(tcb_get_appid(current) != 0)	/* AppID should be 0 */
		return 1;

	unsigned offset = tcb_get_offset(current);
	table = (unsigned*)((unsigned)table | offset);

	switch(type){
		case MON_QOS:
			MMR_MON_PTR_QOS = (unsigned)table;
			break;
		case MON_PWR:
			MMR_MON_PTR_PWR = (unsigned)table;
			break;
		case MON_2:
			MMR_MON_PTR_2 = (unsigned)table;
			break;
		case MON_3:
			MMR_MON_PTR_3 = (unsigned)table;
			break;
		case MON_4:
			MMR_MON_PTR_4 = (unsigned)table;
			break;
		default:
			return 2;
	}

	return 0;
}
