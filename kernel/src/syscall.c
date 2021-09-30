/**
 * 
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
#include "calls.h"
#include "task_location.h"
#include "task_migration.h"
#include "pending_service.h"
#include "interrupts.h"
#include "broadcast.h"

bool schedule_after_syscall;	//!< Signals the HAL syscall to call scheduler

int os_syscall(unsigned int service, unsigned int a1, unsigned int a2, unsigned int a3)
{
	// puts("Syscall called\n");
	schedule_after_syscall = false;
	switch(service){
		case EXIT:
			return os_exit(a1);
		case WRITEPIPE:
			return os_writepipe(a1, a2, a3);
		case READPIPE:
			return os_readpipe(a1, a2, a3);
		case GETTICK:
			return os_get_tick();
		case REALTIME:
			return os_realtime(a1, a2, a3);
		case GETLOCATION:
			return os_get_location();
		case GETID:
			return os_get_id();
		case SCALL_PUTC:
			return os_putc(a1);
		case SCALL_PUTS:
			return os_puts((char*)a1);
		case SCALL_BR_SEND:
			return os_br_send(a1, a2, a3);
		case SCALL_BR_RECEIVE:
			return os_br_receive((uint32_t*)a1);
		case SCALL_MON_PTR:
			return os_mon_ptr((monitor_t*)a1, a2);
		default:
			printf("ERROR: Unknown service %x\n", service);
			return 0;
	}
}

bool os_exit(int status)
{
	schedule_after_syscall = true;

	tcb_t *current = sched_get_current();

	/* Avoid sending a packet while DMNI is busy */
	/* Don't erase task with message in pipe */
	if(MMR_DMNI_SEND_ACTIVE || pipe_is_full(current))
		return false;

	/* Send TASK_TERMINATED */
	tl_send_terminated(current);

	printf("Task id %d terminated with status %d\n", current->id, status);
	MMR_TASK_TERMINATED = current->id;

	sched_clear(current);

	tcb_clear(current);

	return true;
}

bool os_writepipe(unsigned int msg_ptr, int cons_task, bool sync)
{
	tcb_t *current = sched_get_current();

	int cons_addr;
	if(cons_task & 0x80000000){
		/* Message directed to peripheral */
		cons_task &= 0xE000FFFF;
		cons_addr = cons_task;
	} else if(cons_task & 0x10000000){
		/* Message directed to kernel */
		cons_task &= 0x1000FFFF;
		cons_addr = cons_task & 0x0000FFFF;
	} else {
		/* Send to task of the same app */
		cons_task &= 0x000000FF;
		// putsvsv("Trying to write pipe from ", current->id, " to ", cons_task);
		cons_addr = tl_search(current, cons_task);
		cons_task |= (current->id & 0x0000FF00);
		// putsv("Trying to write pipe to addr ", cons_addr);

		/* Check if consumer task is allocated */
		if(cons_addr == -1){
			/* Task is blocked until a TASK_RELEASE packet is received */
			sched_block(current);
			schedule_after_syscall = 1;
			return false;
		}
	}

	/* Points the message in the task page. Address composition: offset + msg address */
	// putsv("Message at virtual address ", msg_ptr);
	message_t *message = (message_t*)(tcb_get_offset(current) | msg_ptr);
	// putsv("Message at physical address ", (int)message);
	// putsv("Value 9 of message = ", message->msg[9]);
	// putsv("Message length = ", message->length);

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

				pipe_transfer(message, msg_dst);

				/* Remove the message request from buffer */
				mr_pop(request);

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
			mr_pop(request);
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
		// putsvsv("Readpipe: trying to read from task ", prod_task, " with address ", prod_addr);
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
		// putsvsv("Readpipe: received DATA_AV from task ", prod_task, " with address ", prod_addr);
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
			/* Get the producer TCB */
			tcb_t *prod_tcb = tcb_search(prod_task);

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
				message[3], 
				message[4], 
				(int*)&message[5]
			);
		case TASK_MIGRATION:
			return os_task_migration(message[1], message[2]);
		case RELEASE_PERIPHERAL:
			return os_release_peripheral();
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

	// putsvsv("Kernel writing pending message to task ", task, " with size ", size);
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
	puts("Peripherals released\n");
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

int os_br_send(uint32_t payload, uint16_t target, uint8_t service)
{
	tcb_t *current = sched_get_current();
	uint16_t producer = tcb_get_id(current);
	
	if(producer >> 8 != 0)	/* AppID should be 0 */
		return 2;

	return !br_send(payload, producer, target, service);
}

int os_br_receive(uint32_t *payload)
{
	if(!MMR_BR_HAS_MESSAGE)
		return 0;

	tcb_t *current = sched_get_current();
	uint32_t *payload_ptr = (uint32_t*)(tcb_get_offset(current) | (unsigned int)payload);

	*payload_ptr = MMR_BR_READ_PAYLOAD;
	return 1;
}

int os_mon_ptr(monitor_t* table, enum MON_TYPE type)
{
	tcb_t *current = sched_get_current();
	
	if(tcb_get_appid(current) != 0)	/* AppID should be 0 */
		return 1;

	uint32_t offset = tcb_get_offset(current);
	table = (uint32_t)table | offset;

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
