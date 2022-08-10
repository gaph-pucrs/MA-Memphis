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

#include <machine/syscall.h>
#include <unistd.h>
#include <errno.h>

#include <memphis.h>

#include "syscall.h"
#include "services.h"
#include "memphis.h"
#include "task_location.h"
#include "task_migration.h"
#include "pending_service.h"
#include "interrupts.h"
#include "broadcast.h"
#include "llm.h"
#include "pending_msg.h"

bool schedule_after_syscall;	//!< Signals the HAL syscall to call scheduler
bool task_terminated;

int os_syscall(
	unsigned arg1, 
	unsigned arg2, 
	unsigned arg3, 
	unsigned arg4, 
	unsigned arg5, 
	unsigned arg6, 
	unsigned arg7, 
	unsigned number
)
{
	// printf("syscall(%d, %d, %d, %d, %d)\n", number, arg1, arg2, arg3, arg4);

	int ret = 0;
	schedule_after_syscall = false;
	task_terminated = false;

	if(sched_check_stack()){
		tcb_t *current = sched_get_current();
		printf(
			"Task id %d aborted due to stack overflow\n", 
			tcb_get_id(current)
		);

		tcb_abort_task(current);

		schedule_after_syscall = true;
		task_terminated = true;
	} else {
		switch(number){
			case SYS_writepipe:
				ret = os_writepipe((void*)arg1, arg2, arg3, arg4);
				break;
			case SYS_readpipe:
				ret = os_readpipe((void*)arg1, arg2, arg3, arg4);
				break;
			case SYS_gettick:
				ret = os_get_tick();
				break;
			case SYS_realtime:
				ret = os_realtime(arg1, arg2, arg3);
				break;
			case SYS_getlocation:
				ret = os_get_location();
				break;
			case SYS_brall:
				ret = os_br_send_all(arg1, arg2);
				break;
			case SYS_brtgt:
				ret = os_br_send_tgt(arg1, arg2, arg3);
				break;
			case SYS_monptr:
				ret = os_mon_ptr((unsigned*)arg1, arg2);
				break;
			case SYS_close:
				ret = os_close(arg1);
				break;
			case SYS_write:
				ret = os_write(arg1, (char*)arg2, arg3);
				break;
			case SYS_fstat:
				ret = os_fstat(arg1, (struct stat*)arg2);
				break;
			case SYS_exit:
				ret = os_exit(arg1);
				break;
			case SYS_getpid:
				ret = os_getpid();
				break;
			case SYS_brk:
				ret = os_brk(arg1);
				break;
			default:
				printf("ERROR: Unknown syscall %x\n", number);
				ret = 0;
				break;
		}
	}

	if(schedule_after_syscall)
		sched_run();

	return ret;
}

bool os_exit(int status)
{
	schedule_after_syscall = true;

	tcb_t *current = sched_get_current();

	printf("Task id %d terminated with status %d\n", current->id, status);

	if(tcb_get_opipe(current) != NULL){
		/* Don't erase task with message in pipe */
		tcb_set_called_exit(current);
		sched_set_wait_request(current);
		schedule_after_syscall = true;
		return false;
	}
	
	task_terminated = true;

	tcb_terminate(current);

	return true;
}

int os_writepipe(void *buf, size_t size, int cons_task, bool sync)
{
	tcb_t *current = sched_get_current();

	if((cons_task & 0xFFFF0000) && current->id >> 8 != 0){
		puts(
			"ERROR: Unauthorized message to kernel/peripheral from task with app id > 0\n"
		);
		return -EACCES;
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
			return -EAGAIN;
		}
	}

	/* Points the message in the task page. Address composition: offset + msg address */
	// printf("Message at virtual address %x\n", msg_ptr);
	if(!buf){
		printf("ERROR: message pointer is null\n");
		return -EINVAL;
	}

	buf = (void*)((unsigned)buf | (unsigned)tcb_get_offset(current));
	// printf("Message at physical address %x\n", (int)message);
	// printf("Message length = %d\n", message->length);

	/* Searches if there is a message request to the produced message */
	message_request_t *request = mr_peek(current, cons_task);

	if(request){	/* Message request found! */
		// puts("MR Found!\n");
		if(request->requester_address == MMR_NI_CONFIG){ 
			/* Local consumer */
			if(cons_task & 0x10000000){
				/* Message directed to kernel. No TCB to write to */
				/* This should NEVER happen because it means the kernel made a request without receiving a DATA_AV */
				puts(
					"ERROR: Kernel made a request without receiving DATA_AV!"
				);
				return -EBADMSG;
			} else {
				/* Writes to the consumer page address */
				tcb_t *cons_tcb = tcb_search(cons_task);

				if(!cons_tcb){
					puts("ERROR: consumer tcb not found");
					return -EBADMSG;
				}

				ipipe_t *dst = tcb_get_ipipe(cons_tcb);

				if(dst == NULL){
					puts(
						"ERROR: there is no buffer allocated to transfer message"
					);
					return -EBADMSG;
				}

				int result = ipipe_transfer(
					dst, 
					tcb_get_offset(cons_tcb), 
					buf, 
					size
				);

				if(result == -1){
					puts(
						"ERROR: destination pointer not found in ipipe"
					);
					return -EBADMSG;
				}

				if(result != size){
					puts(
						"ERROR: destination buffer not big enough to receive message"
					);
					return -EBADMSG;
				}

				/* Remove the input pipe from TCB */
				tcb_destroy_ipipe(cons_tcb);

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
			if(MMR_DMNI_SEND_ACTIVE || tcb_get_opipe(current)){
				schedule_after_syscall = true;
				return -EAGAIN;
			}

			opipe_t *opipe = tcb_create_opipe(current);

			if(opipe == NULL){
				puts("ERROR: not enough memory for opipe");
				return -ENOMEM;
			}

			/* Insert the message in the pipe to avoid overwrite by task */
			int prod_task = sched_get_current_id();
			int result = opipe_push(opipe, buf, size, prod_task, cons_task);

			if(result != size){
				puts("ERROR: not enough memory for opipe message");
				return -ENOMEM;
			}

			/* Send through NoC */
			// void *out_msg = opipe_pop(pipe, NULL, NULL);
			opipe_send(
				opipe, 
				sched_get_current_id(), 
				request->requester_address
			);
			// puts("Sent through NoC\n");
			
			/* Remove the message request from buffer */
			mr_pop(request, current->id);
			// puts("Request popped\n");

			tcb_destroy_opipe(current);
		}
	} else if(!tcb_get_opipe(current) && !MMR_DMNI_SEND_ACTIVE){	/* Pipe is free */
		// puts("MR NOT found!\n");
		if(sync){
			// puts("Sync!\n");
			int prod_task = sched_get_current_id();
			if(cons_addr == MMR_NI_CONFIG){
				/* Local consumer */
				if(cons_task & 0x10000000){
					/* Message directed to kernel. No TCB to write to */
					/* We can bypass the need to kernel answer if a request */
					schedule_after_syscall = os_kernel_syscall(buf, size >> 2);
					return 0;
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
		opipe_t *opipe = tcb_create_opipe(current);

		int prod_task = sched_get_current_id();
		int result = opipe_push(opipe, buf, size, prod_task, cons_task);

		if(result != size){
			puts("ERROR: not enough memory for opipe message");
			return -ENOMEM;
		}
		// puts("Pushed to pipe!");
	} else {
		/* DMNI busy sending message or pipe full */
		if(tcb_get_opipe(current)){
			// puts("Request not found and pipe is full. Will wait until consumed!\n");
			/* In this case, we must wait for a message request to release the pipe */
			sched_set_wait_request(current);
			schedule_after_syscall = 1;
		} /* else: the DMNI is sending the older message. Must retry */
		
		return -EAGAIN;
	}

	return size;
}

int os_readpipe(void *buf, size_t size, int prod_task, bool sync)
{
	// puts("Calling readpipe");
	tcb_t *current = sched_get_current();

	ipipe_t *ipipe = tcb_get_ipipe(current);
	// printf("ipipe is at addr %p\n", ipipe);
	if(ipipe != NULL && ipipe_is_read(ipipe)){
		// puts("Returning from readpipe");
		int ret = ipipe_get_size(ipipe);
		tcb_destroy_ipipe(current);
		return ret;
	}

	if(buf == NULL){
		puts("ERROR: msg is null");
		return -EINVAL;
	}

	int cons_task = sched_get_current_id();

	int prod_addr;
	if(!sync){	/* Not synced READ must define the producer */
		prod_task |= (cons_task & 0xFF00);
		prod_addr = tl_search(current, prod_task);
		// printf("Trying to read from task %x at address %x\n", prod_task, prod_addr);

		/* Test if the producer task is not allocated */
		if(prod_addr == -1){
			/* Task is blocked until its a TASK_RELEASE packet */
			sched_block(current);
			schedule_after_syscall = 1;
			return -EAGAIN;
		}

		// printf("Readpipe: trying to read from task %x with address %x\n", prod_task, prod_addr);

	} else {
		/* Synced READ, must see what applications have data available for it */
		data_av_t *data_av = data_av_peek(current);
		
		if(!data_av){	/* No data available for requesting */
			/* Block task and wait for DATA_AV packet */
			sched_set_wait_data_av(current);
			schedule_after_syscall = 1;
			return -EAGAIN;
		}

		prod_task = data_av->requester;
		prod_addr = data_av->requester_addr;
		// printf("Readpipe: received DATA_AV from task %x with address %x\n", prod_task, prod_addr);
	}

	if(prod_addr == MMR_NI_CONFIG){	/* Local producer */
		if(prod_task & MEMPHIS_KERNEL_MSG){
			// puts("Received DATA_AV from LOCAL Kernel!\n");
			/* Message from Kernel. No request needed */
			/* Search for the kernel-produced message */
			opipe_t *pending = pend_msg_find(cons_task);

			/* Store it like a MESSAGE_DELIVERY */
			buf = (void*)((unsigned)buf | (unsigned)tcb_get_offset(current));
			
			// putsv("Message length is ", msg->size);
			// putsv("First word is ", msg->message[0]);
			int result = opipe_transfer(pending, buf, size);

			if(result <= 0){
				puts("ERROR: could not read from pipe");
				return -EBADMSG;
			}

			opipe_pop(pending);
			pend_msg_remove(pending);

			/* Remove pending DATA_AV */
			data_av_pop(current);

			/* Add a new data available if kernel has message to this task */
			if(pend_msg_find(cons_task) != NULL){
				/* Add a new to the last position of the FIFO */
				data_av_insert(
					current, 
					MEMPHIS_KERNEL_MSG | MMR_NI_CONFIG, 
					MMR_NI_CONFIG
				);
			}

			return size;
		} else {
			// puts("Local producer\n");
			/* Get the producer TCB */
			tcb_t *prod_tcb = tcb_search(prod_task);

			if(!prod_tcb){
				puts("ERROR: PROD TCB NOT FOUND ON READPIPE");
				return -EBADMSG;
			}

			/* Searches if the message is in PIPE (local producer) */
			opipe_t *opipe = tcb_get_opipe(current);

			if(opipe == NULL || opipe_get_cons_task(opipe) != cons_task){
				/* Stores the request into the message request table */
				mr_insert(prod_tcb, cons_task, MMR_NI_CONFIG);

			} else {
				/* Message was found in pipe, writes to the consumer page address (local producer) */
				buf = (void*)((unsigned)buf | (unsigned)tcb_get_offset(current));

				int result = opipe_transfer(opipe, buf, size);

				if(result <= 0){
					puts("ERROR: could not read from pipe");
					return -EBADMSG;
				}

				opipe_pop(opipe);
				tcb_destroy_opipe(prod_tcb);

				if(sched_is_waiting_request(prod_tcb)){
					sched_release_wait(prod_tcb);
					if(tcb_has_called_exit(prod_tcb)){
						tcb_terminate(prod_tcb);
					}
				}

				return result;
			}
		}
	} else { /* Remote producer : Sends the message request */
		// puts("Remote producer\n");
		/* Deadlock avoidance: avoids to send a packet when the DMNI is busy in send process */
		if(MMR_DMNI_SEND_ACTIVE){
			schedule_after_syscall = true;
			return -EAGAIN;
		}
		
		if(sync){
			/* DATA_AV is processed, erase it */
			data_av_pop(current);
		}

		/* Send the message request through NoC */
		mr_send(prod_task, cons_task, prod_addr, MMR_NI_CONFIG);
		// puts("Sent request");
	}

	/* Stores the message pointer to receive */
	ipipe = tcb_create_ipipe(current);
	// printf("Allocated ipipe at %p\n", current->pipe_in);
	ipipe_set(ipipe, buf, size);
	// printf("Set ipipe to %p size %d\n", ipipe->buf, ipipe->size);

	/* Sets task as waiting blocking its execution, it will execute again when the message is produced by a WRITEPIPE or incoming MSG_DELIVERY */
	sched_set_wait_delivery(current);
	schedule_after_syscall = 1;

	return -EAGAIN;
}

unsigned int os_get_tick()	
{	
	return MMR_TICK_COUNTER;	
}

int os_realtime(unsigned int period, int deadline, unsigned int exec_time)
{
	tcb_t *current = sched_get_current();
	sched_real_time_task(current, period, deadline, exec_time);

	schedule_after_syscall = 1;

	return 0;
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
		default:
			printf(
				"ERROR: Unknown service %x inside MESSAGE_DELIVERY\n", 
				message[0]
			);
			return false;
	}
}

bool os_kernel_writepipe(void *buf, size_t size, int cons_task, int cons_addr)
{
	/* Send data available only if target task hasn't received data available from this source */
	bool send_data_av = (pend_msg_find(cons_task) == NULL);

	/* Avoid overwriting pending messages */
	while(MMR_DMNI_SEND_ACTIVE);

	// printf("Kernel writing pending message to task %d with size %d\n", task, size);
	/* Insert message in kernel output message buffer */
	int result = pend_msg_push(buf, size, cons_task);

	if(result != size){
		puts("ERROR: cant write to kernel pipe");
		while(1);
	}

	if(send_data_av){
		/* Check if local consumer / migrated task */
		tcb_t *cons_tcb = NULL;
		if(cons_addr == MMR_NI_CONFIG){
			cons_tcb = tcb_search(cons_task);
			if(!cons_tcb)
				cons_addr = tm_get_migrated_addr(cons_task);
		}

		if(cons_tcb){
			/* Insert the packet to TCB */
			data_av_insert(
				cons_tcb, 
				MEMPHIS_KERNEL_MSG | MMR_NI_CONFIG, 
				MMR_NI_CONFIG
			);

			/* If the consumer task is waiting for a DATA_AV, release it */
			if(sched_is_waiting_data_av(cons_tcb)){
				sched_release_wait(cons_tcb);
				return sched_is_idle();
			}
		} else {
			/* Send data available to the right processor */
			data_av_send(
				cons_task, 
				MEMPHIS_KERNEL_MSG | MMR_NI_CONFIG, 
				cons_addr, 
				MMR_NI_CONFIG
			);
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

int os_getpid()
{
	return sched_get_current_id();
}

int os_br_send_all(uint32_t payload, uint8_t ksvc)
{
	int producer = sched_get_current_id();
	
	if(producer >> 8 != 0)	/* AppID should be 0 */
		return -EINVAL;

	br_packet_t packet;
	packet.service = ksvc;
	packet.src_id = producer;
	packet.payload = payload;

	if(!br_send(&packet, MMR_NI_CONFIG, BR_SVC_ALL)){
		schedule_after_syscall = true;
		return -EAGAIN;
	}
		
	packet.src_addr = MMR_NI_CONFIG;
	schedule_after_syscall = os_handle_broadcast(&packet);
	return 0;
}

int os_br_send_tgt(uint32_t payload, uint16_t target, uint8_t ksvc)
{
	int producer = sched_get_current_id();
	
	if(producer >> 8 != 0)	/* AppID should be 0 */
		return -EINVAL;

	br_packet_t packet;
	packet.service = ksvc;
	packet.src_id = producer;
	packet.payload = payload;

	if(target == MMR_NI_CONFIG){
		packet.src_addr = MMR_NI_CONFIG;
		schedule_after_syscall = os_handle_broadcast(&packet);
		return 0;
	}

	if(!br_send(&packet, target, BR_SVC_TGT)){
		schedule_after_syscall = true;
		return -EAGAIN;
	}

	return 0;
}

int os_mon_ptr(unsigned* table, enum MONITOR_TYPE type)
{
	tcb_t *current = sched_get_current();
	
	if(tcb_get_appid(current) != 0)	/* AppID should be 0 */
		return -EINVAL;

	if(table == NULL){
		printf("ERROR: Table is null.\n");
		return -EINVAL;
	}

	table = (unsigned*)((unsigned)table | (unsigned)tcb_get_offset(current));

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
			return -EINVAL;
	}

	return 0;
}

int os_brk(unsigned addr)
{
	// printf("brk(%u)\n", addr);
	tcb_t *current = sched_get_current();

	unsigned heap_end = tcb_get_heap_end(current);

	if(addr == 0 || addr == heap_end){
		// printf("Returning %u\n", heap_end);
		return heap_end;
	}

	// if(addr < heap_end)
	// 	return addr;

	unsigned sp = tcb_get_sp(current);

	if((unsigned)addr > sp){
		fprintf(
			stderr, 
			"Heap and stack collision in task %d\n", 
			tcb_get_id(current)
		);
		return -1;
	}

	// printf("Growing heap from %u to %u\n", heap_end, (unsigned)addr);

	tcb_set_brk(current, addr);

	return addr;
}

int os_write(int file, char *buf, int nbytes)
{
	tcb_t *current = sched_get_current();

	if(file != STDOUT_FILENO && file != STDERR_FILENO)
		return -EBADF;

	if(buf == NULL){
		printf("ERROR: buffer is null\n");
		return -EINVAL;
	}

	buf = (char*)((unsigned)tcb_get_offset(current) | (unsigned)buf);


	int rv = 0;
	if(nbytes == 1 && buf[0] == '\n'){
		rv = write(file, buf, nbytes);
	} else {
		int id = sched_get_current_id();
		int addr = MMR_NI_CONFIG;

		printf("$$$_%dx%d_%d_%d_", addr >> 8, addr & 0xFF, id >> 8, id & 0xFF);
		fflush(stdout);

		rv = write(file, buf, nbytes);
	}

	if(rv == -1)
		return -errno;

	return rv;
}

int os_fstat(int file, struct stat *st)
{
	tcb_t *current = sched_get_current();

	if(st == NULL){
		printf("ERROR: st is null");
		return false;
	}

	st = (struct stat*)((unsigned)tcb_get_offset(current) | (unsigned)st);
	int ret = fstat(file, st);

	if(ret == -1)
		return -errno;

	return ret;
}

int os_close(int file)
{
	return -EBADF;
}
