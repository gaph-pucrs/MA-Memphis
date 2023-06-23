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

#include "syscall.h"

#include <stdio.h>
// #include <string.h>

#include <machine/syscall.h>
#include <unistd.h>
#include <errno.h>

#include <memphis/services.h>

#include "interrupts.h"
#include "broadcast.h"
#include "pending_msg.h"
#include "pending_service.h"
#include "task_migration.h"
#include "mmr.h"
#include "dmni.h"

bool schedule_after_syscall;	//!< Signals the HAL syscall to call scheduler
bool task_terminated;

tcb_t *sys_syscall(
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

	tcb_t *current = sched_get_current_tcb();

	if(tcb_check_stack(current)){
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
				ret = sys_writepipe(current, (void*)arg1, arg2, arg3, arg4);
				break;
			case SYS_readpipe:
				ret = sys_readpipe(current, (void*)arg1, arg2, arg3, arg4);
				break;
			case SYS_gettick:
				ret = sys_get_tick();
				break;
			case SYS_realtime:
				ret = sys_realtime(current, arg1, arg2, arg3);
				break;
			case SYS_getlocation:
				ret = sys_get_location();
				break;
			case SYS_brall:
				ret = sys_br_send_all(current, arg1, arg2);
				break;
			case SYS_brtgt:
				ret = sys_br_send_tgt(current, arg1, arg2, arg3);
				break;
			case SYS_monptr:
				ret = sys_mon_ptr(current, (unsigned*)arg1, arg2);
				break;
			case SYS_getctx:
				ret = sys_get_ctx(current, (mctx_t*)arg1);
				break;
			case SYS_halt:
				ret = sys_end_simulation(current);
				break;
			case SYS_close:
				ret = sys_close(arg1);
				break;
			case SYS_write:
				ret = sys_write(current, arg1, (char*)arg2, arg3);
				break;
			case SYS_fstat:
				ret = sys_fstat(current, arg1, (struct stat*)arg2);
				break;
			case SYS_exit:
				ret = sys_exit(current, arg1);
				break;
			case SYS_getpid:
				ret = sys_getpid(current);
				break;
			case SYS_brk:
				ret = sys_brk(current, (void*)arg1);
				break;
			case SYS_sendraw:
				ret = sys_sendraw(current, (void*)arg1, arg2);
				break;
			default:
				printf("ERROR: Unknown syscall %x\n", number);
				ret = 0;
				break;
		}
	}
	
	if(!task_terminated){
		// printf("Setting return value %d\n", ret);
		tcb_set_ret(current, ret);
	}

	schedule_after_syscall |= (MMR_IRQ_STATUS & MMR_IRQ_MASK & IRQ_SCHEDULER);
	if(schedule_after_syscall){
		sched_run();
		return sched_get_current_tcb();
	}

	return current;
}

int sys_exit(tcb_t *tcb, int status)
{
	printf("Task id %d terminated with status %d\n", tcb_get_id(tcb), status);

	schedule_after_syscall = true;

	if(tcb_get_opipe(tcb) != NULL){
		/* Don't erase task with message in pipe */
		tcb_set_called_exit(tcb);
		sched_t *sched = tcb_get_sched(tcb);
		sched_set_wait_msgreq(sched);
		return -EAGAIN;
	}

	tcb_terminate(tcb);
	task_terminated = true;

	return 0;
}

int sys_writepipe(tcb_t *tcb, void *buf, size_t size, int cons_task, bool sync)
{
	const int prod_task = tcb_get_id(tcb);

	if((cons_task & 0xFFFF0000) && prod_task >> 8 != 0){
		puts(
			"ERROR: Unauthorized message to kernel/peripheral from task with app id > 0"
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

		app_t *app = tcb_get_app(tcb);
		if(app == NULL){
			puts("ERROR: app not found");
			return -EINVAL;
		}

		cons_addr = app_get_address(app, cons_task);
		if(cons_addr == -1){
			puts("ERROR: task not found");
			return -EINVAL;
		}

		cons_task |= (prod_task & 0x0000FF00);
	}

	/* Points the message in the task page. Address composition: offset + msg address */
	if(buf == NULL){
		puts("ERROR: message pointer is null");
		return -EINVAL;
	}

	/* Sets real address of message buffer */
	buf = (void*)((unsigned)buf | (unsigned)tcb_get_offset(tcb));

	/* Searches if there is a message request to the produced message */
	list_t *msgreqs = tcb_get_msgreqs(tcb);
	tl_t *request = tl_find(msgreqs, cons_task);

	if(request != NULL){
		/* Message request found! */
		int req_addr = tl_get_addr(request);
		if(req_addr == MMR_NI_CONFIG){
			/* Local consumer */
			if(cons_task & MEMPHIS_KERNEL_MSG){
				/* Message directed to kernel. No TCB to write to */
				/* This should NEVER happen because it means the kernel made a request without receiving a DATA_AV */
				puts(
					"ERROR: Kernel made a request without receiving DATA_AV!"
				);
				return -EBADMSG;
			} else {
				/* Writes to the consumer page address */
				tcb_t *cons_tcb = tcb_find(cons_task);

				if(cons_tcb == NULL){
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

				if(result != size){
					puts(
						"ERROR: faulty destination ipipe"
					);
					return -EBADMSG;
				}

				/* Remove the message request from buffer */
				tl_remove(msgreqs, request);

				/* Release consumer task */
				sched_t *sched = tcb_get_sched(cons_tcb);
				sched_release_wait(sched);

				if(tcb_need_migration(cons_tcb)){
					tm_migrate(cons_tcb);
					schedule_after_syscall = 1;
				}
			}
		} else {
			/* Remote consumer */
			/* Send a MESSAGE_DELIVERY */

			/* Deadlock avoidance: avoid sending a packet when the DMNI is busy */
			/* Also, don't send a message if the previous is still in pipe */
			if(tcb_get_opipe(tcb) != NULL){
				sched_t *sched = tcb_get_sched(tcb);
				sched_set_wait_msgreq(sched);
				schedule_after_syscall = true;
				return -EAGAIN;
			}

			if(MMR_DMNI_SEND_ACTIVE){
				schedule_after_syscall = true;
				return -EAGAIN;
			}

			opipe_t *opipe = tcb_create_opipe(tcb);

			if(opipe == NULL){
				puts("ERROR: not enough memory for opipe");
				return -ENOMEM;
			}

			/* Insert the message in the pipe to avoid overwrite by task */
			int result = opipe_push(opipe, buf, size, prod_task, cons_task);

			if(result != size){
				puts("ERROR: not enough memory for opipe message");
				return -ENOMEM;
			}

			/* Send through NoC */
			opipe_send(
				opipe, 
				prod_task, 
				req_addr
			);

			tcb_destroy_opipe(tcb);
			
			/* Remove the message request from buffer */
			tl_remove(msgreqs, request);
		}
	} else if(tcb_get_opipe(tcb) == NULL){
		/* Pipe is free */
		if(sync){
			if(cons_addr == MMR_NI_CONFIG){
				/* Local consumer */
				if(cons_task & MEMPHIS_KERNEL_MSG){
					/* Message directed to kernel. No TCB to write to */
					/* We can bypass the need to kernel answer if a request */
					schedule_after_syscall = sys_kernel_syscall(buf, size);
					return 0;
				} else {
					/* Insert a DATA_AV to consumer table */
					tcb_t *cons_tcb = tcb_find(cons_task);

					/* Insert DATA_AV to the consumer TCB */
					list_t *davs = tcb_get_davs(cons_tcb);
					tl_t *dav = tl_emplace_back(davs, prod_task, MMR_NI_CONFIG);

					if(dav == NULL){
						puts("ERROR: could not allocate memory for DAV entry");
						return -EAGAIN;
					}
					
					/* If consumer waiting for a DATA_AV, release the task */
					sched_t *sched = tcb_get_sched(cons_tcb);
					if(sched_is_waiting_dav(sched))
						sched_release_wait(sched);
				}
			} else {
				/* Send DATA_AV to consumer PE */
				tl_t dav;
				tl_set(&dav, prod_task, MMR_NI_CONFIG);
				
				tl_send_dav(&dav, cons_task, cons_addr);
			}
		}

		/* Please use SSend to send Kernel/Peripheral messages or it can be stuck in pipe forever */
		/* Store message in Pipe. Will be sent when a REQUEST is received */
		opipe_t *opipe = tcb_create_opipe(tcb);

		int result = opipe_push(opipe, buf, size, prod_task, cons_task);

		if(result != size){
			puts("ERROR: not enough memory for opipe message");
			return -ENOMEM;
		}
		// puts("Pushed to pipe!");
	} else {
		/* Pipe full */

		// puts("Request not found and pipe is full. Will wait until consumed!");
		/* In this case, we must wait for a message request to release the pipe */
		sched_t *sched = tcb_get_sched(tcb);
		sched_set_wait_msgreq(sched);
		schedule_after_syscall = 1;
		
		return -EAGAIN;
	}

	return size;
}

int sys_sendraw(tcb_t *tcb, void *buf, size_t size)
{
	// puts("[Syscall] Entering sendraw routine.");
	
	if(size < 13){
		puts("ERROR: invalid packet size");
		return -EINVAL;
	}

	/* Points the message in the task page. Address composition: offset + msg address */
	if(buf == NULL){
		puts("ERROR: message pointer is null");
		return -EINVAL;
	}

	/* Sets real address of message buffer */
	buf = (void*)((unsigned)buf | (unsigned)tcb_get_offset(tcb));

	dmni_send_raw(buf, size);

	return 0;
}

int sys_readpipe(tcb_t *tcb, void *buf, size_t size, int prod_task, bool sync)
{
	// puts("Calling readpipe");
	ipipe_t *ipipe = tcb_get_ipipe(tcb);
	// printf("ipipe is at addr %p\n", ipipe);
	if(ipipe != NULL){
		if(ipipe_is_read(ipipe)){
			// puts("Returning from readpipe");
			int ret = ipipe_get_size(ipipe);
			tcb_destroy_ipipe(tcb);
			return ret;
		}

		/* This should never happen, but is here just in case */
		puts("ERROR: ipipe is not null but was not read");
		return -EAGAIN;
	}

	if(buf == NULL){
		puts("ERROR: msg is null");
		return -EINVAL;
	}

	const int cons_task = tcb_get_id(tcb);

	int prod_addr;
	if(!sync){	/* Not synced READ must define the producer */
		prod_task &= 0x000000FF;

		app_t *app = tcb_get_app(tcb);
		if(app == NULL){
			puts("ERROR: app not found");
			return -EINVAL;
		}

		prod_addr = app_get_address(app, prod_task);
		if(prod_addr == -1){
			puts("ERROR: task not found");
			return -EINVAL;
		}

		prod_task |= (cons_task & 0xFF00);

		// printf("Trying to read from task %x at address %x\n", prod_task, prod_addr);
		// printf("Readpipe: trying to read from task %x with address %x\n", prod_task, prod_addr);
	} else {
		/* Synced READ, must see what applications have data available for it */
		list_t *davs = tcb_get_davs(tcb);
		list_entry_t *front = list_front(davs);
		tl_t *dav = NULL;
		
		if(front != NULL)
			dav = list_get_data(front);
		
		if(dav == NULL){	/* No data available for requesting */
			/* Block task and wait for DATA_AV packet */
			sched_t *sched = tcb_get_sched(tcb);
			sched_set_wait_dav(sched);
			schedule_after_syscall = 1;
			return -EAGAIN;
		}

		prod_task = tl_get_task(dav);
		prod_addr = tl_get_addr(dav);
		// printf("Readpipe: received DATA_AV from task %x with address %x\n", prod_task, prod_addr);
	}

	if(prod_addr == MMR_NI_CONFIG){
		/* Local producer */
		if(prod_task & MEMPHIS_KERNEL_MSG){
			// puts("Received DATA_AV from LOCAL Kernel!\n");
			/* Message from Kernel. No request needed */
			/* Search for the kernel-produced message */
			opipe_t *pending = pmsg_find(cons_task);

			/* Store it like a MESSAGE_DELIVERY */
			buf = (void*)((unsigned)buf | (unsigned)tcb_get_offset(tcb));
			
			// putsv("Message length is ", msg->size);
			// putsv("First word is ", msg->message[0]);
			int result = opipe_transfer(pending, buf, size);

			if(result <= 0){
				puts("ERROR: could not read from pipe");
				return -EBADMSG;
			}

			opipe_pop(pending);
			pmsg_remove(pending);

			/* Remove pending DATA_AV */
			list_t *davs = tcb_get_davs(tcb);
			tl_t *dav = list_get_data(list_front(davs));
			tl_remove(davs, dav);

			/* Add a new data available if kernel has message to this task */
			if(pmsg_find(cons_task) != NULL){
				/* Add a new to the last position of the FIFO */
				tl_emplace_back(davs, prod_task, prod_addr);
			}

			return size;
		} else {
			/* Get the producer TCB */
			tcb_t *prod_tcb = tcb_find(prod_task);

			if(prod_tcb == NULL){
				puts("ERROR: PROD TCB NOT FOUND ON READPIPE");
				return -EBADMSG;
			}

			/* Searches if the message is in PIPE (local producer) */
			opipe_t *opipe = tcb_get_opipe(prod_tcb);

			if(opipe == NULL || opipe_get_cons_task(opipe) != cons_task){
				/* Stores the request into the message request table */
				list_t *msgreqs = tcb_get_msgreqs(prod_tcb);
				tl_emplace_back(msgreqs, cons_task, MMR_NI_CONFIG);
			} else {
				/* Message was found in pipe, writes to the consumer page address (local producer) */
				buf = (void*)((unsigned)buf | (unsigned)tcb_get_offset(tcb));

				int result = opipe_transfer(opipe, buf, size);

				if(result <= 0){
					puts("ERROR: could not read from pipe");
					return -EBADMSG;
				}

				opipe_pop(opipe);
				tcb_destroy_opipe(prod_tcb);

				sched_t *sched = tcb_get_sched(prod_tcb);

				if(sched_is_waiting_msgreq(sched)){
					sched_release_wait(sched);
					if(tcb_has_called_exit(prod_tcb))
						tcb_terminate(prod_tcb);
				}

				return result;
			}
		}
	} else { /* Remote producer : Sends the message request */
		// puts("Remote producer\n");
		
		if(sync){
			/* DATA_AV is processed, erase it */
			list_t *davs = tcb_get_davs(tcb);
			tl_t *dav = list_get_data(list_front(davs));
			tl_remove(davs, dav);
		}

		/* Send the message request through NoC */
		tl_t msgreq;
		tl_set(&msgreq, cons_task, MMR_NI_CONFIG);

		tl_send_msgreq(&msgreq, prod_task, prod_addr);
		// puts("Sent request");
	}

	/* Stores the message pointer to receive */
	ipipe = tcb_create_ipipe(tcb);
	
	if(ipipe == NULL){
		puts("FATAL");
		while(true);
	}
	// printf("Allocated ipipe at %p\n", current->pipe_in);
	ipipe_set(ipipe, buf, size);
	// printf("Set ipipe to %p size %d\n", ipipe->buf, ipipe->size);

	/* Sets task as waiting blocking its execution, it will execute again when the message is produced by a WRITEPIPE or incoming MSG_DELIVERY */
	sched_t *sched = tcb_get_sched(tcb);
	sched_set_wait_msgdlvr(sched);
	schedule_after_syscall = 1;

	return -EAGAIN;
}

unsigned int sys_get_tick()	
{	
	return MMR_TICK_COUNTER;	
}

int sys_realtime(tcb_t *tcb, unsigned period, int deadline, unsigned exec_time)
{
	sched_t *sched = tcb_get_sched(tcb);
	sched_real_time_task(sched, period, deadline, exec_time);

	schedule_after_syscall = 1;

	return 0;
}

bool sys_kernel_syscall(unsigned *message, int length)
{
	/* Process it like a syscall */
	switch(message[0]){
		case TASK_RELEASE:
			// putsv("will call release with ntasks=", message[4]);
			return isr_task_release(
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

bool sys_kernel_writepipe(void *buf, size_t size, int cons_task, int cons_addr)
{
	/* Send data available only if target task hasn't received data available from this source */
	bool send_data_av = (pmsg_find(cons_task) == NULL);

	// printf("Kernel writing pending message to task %d with size %d\n", task, size);
	/* Insert message in kernel output message buffer */
	int result = pmsg_emplace_back(buf, size, cons_task);

	if(result != size){
		puts("FATAL: cant write to kernel pipe");
		while(1);
	}

	if(send_data_av){
		/* Check if local consumer / migrated task */
		tcb_t *cons_tcb = NULL;
		if(cons_addr == MMR_NI_CONFIG){
			cons_tcb = tcb_find(cons_task);
			if(cons_tcb == NULL){
				tl_t *tl = tm_find(cons_task);
				if(tl == NULL){
					puts("FATAL: task migrated not found");
					while(true);
				}
				cons_addr = tl_get_addr(tl);
			}
		}

		if(cons_tcb != NULL){
			/* Insert the packet to TCB */
			list_t *davs = tcb_get_davs(cons_tcb);
			tl_emplace_back(davs, MEMPHIS_KERNEL_MSG | MMR_NI_CONFIG, MMR_NI_CONFIG);

			/* If the consumer task is waiting for a DATA_AV, release it */
			sched_t *sched = tcb_get_sched(cons_tcb);
			if(sched_is_waiting_dav(sched)){
				sched_release_wait(sched);
				return sched_is_idle();
			}
		} else {
			/* Send data available to the right processor */
			tl_t dav;
			tl_set(&dav, MEMPHIS_KERNEL_MSG | MMR_NI_CONFIG, MMR_NI_CONFIG);

			tl_send_dav(&dav, cons_task, cons_addr);
		}
	}

	return false;
}

bool sys_release_peripheral()
{
	MMR_MEM_REG_PERIPHERALS = 1;
	// puts("Peripherals released\n");
	return false;
}

int sys_get_location()
{
	return MMR_NI_CONFIG;
}

int sys_getpid(tcb_t *tcb)
{
	return tcb_get_id(tcb);
}

int sys_br_send_all(tcb_t *tcb, uint32_t payload, uint8_t ksvc)
{
	int prod_task = tcb_get_id(tcb);
	
	if(prod_task >> 8 != 0)	/* AppID should be 0 */
		return -EINVAL;

	bcast_t packet;
	packet.service = ksvc;
	packet.src_id = prod_task;
	packet.payload = payload;

	if(!bcast_send(&packet, MMR_NI_CONFIG, BR_SVC_ALL)){
		schedule_after_syscall = true;
		return -EAGAIN;
	}
		
	packet.src_addr = MMR_NI_CONFIG;
	schedule_after_syscall = isr_handle_broadcast(&packet);
	return 0;
}

int sys_br_send_tgt(tcb_t *tcb, uint32_t payload, uint16_t target, uint8_t ksvc)
{
	int prod_task = tcb_get_id(tcb);
	
	if(prod_task >> 8 != 0)	/* AppID should be 0 */
		return -EINVAL;

	bcast_t packet;
	packet.service = ksvc;
	packet.src_id = prod_task;
	packet.payload = payload;

	if(target == MMR_NI_CONFIG){
		packet.src_addr = MMR_NI_CONFIG;
		schedule_after_syscall = isr_handle_broadcast(&packet);
		return 0;
	}

	if(!bcast_send(&packet, target, BR_SVC_TGT)){
		schedule_after_syscall = true;
		return -EAGAIN;
	}

	return 0;
}

int sys_mon_ptr(tcb_t *tcb, unsigned* table, enum MONITOR_TYPE type)
{
	if(tcb_get_id(tcb) >> 8 != 0)	/* AppID should be 0 */
		return -EINVAL;

	if(table == NULL){
		printf("ERROR: Table is null.\n");
		return -EINVAL;
	}

	table = (unsigned*)((unsigned)table | (unsigned)tcb_get_offset(tcb));

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

int sys_brk(tcb_t *tcb, void *addr)
{
	// printf("brk(%u)\n", addr);
	void *heap_end = tcb_get_heap_end(tcb);

	if(addr == NULL || addr == heap_end){
		// printf("Returning %u\n", heap_end);
		return (int)heap_end;
	}

	// if(addr < heap_end)
	// 	return addr;

	unsigned sp = tcb_get_sp(tcb);

	if((unsigned)addr > sp){
		fprintf(
			stderr, 
			"Heap and stack collision in task %d\n", 
			tcb_get_id(tcb)
		);
		return -1;
	}

	// printf("Growing heap from %u to %u\n", heap_end, (unsigned)addr);

	tcb_set_brk(tcb, addr);

	return (int)addr;
}

int sys_write(tcb_t *tcb, int file, char *buf, int nbytes)
{
	if(file != STDOUT_FILENO && file != STDERR_FILENO)
		return -EBADF;

	if(buf == NULL){
		printf("ERROR: buffer is null\n");
		return -EINVAL;
	}

	buf = (char*)((unsigned)tcb_get_offset(tcb) | (unsigned)buf);


	int rv = 0;
	if(nbytes == 1 && buf[0] == '\n'){
		rv = write(file, buf, nbytes);
	} else {
		int id = tcb_get_id(tcb);
		int addr = MMR_NI_CONFIG;

		printf("$$$_%dx%d_%d_%d_", addr >> 8, addr & 0xFF, id >> 8, id & 0xFF);
		fflush(stdout);

		rv = write(file, buf, nbytes);
	}

	if(rv == -1)
		return -errno;

	return rv;
}

int sys_fstat(tcb_t *tcb, int file, struct stat *st)
{
	if(st == NULL){
		printf("ERROR: st is null");
		return false;
	}

	st = (struct stat*)((unsigned)tcb_get_offset(tcb) | (unsigned)st);
	int ret = fstat(file, st);

	if(ret == -1)
		return -errno;

	return ret;
}

int sys_close(int file)
{
	return -EBADF;
}

int sys_get_ctx(tcb_t *tcb, mctx_t *ctx)
{
	mctx_t *real_ptr = (mctx_t*)((unsigned)tcb_get_offset(tcb) | (unsigned)ctx);
	real_ptr->PE_X_CNT = MMR_N_PE_X;
	real_ptr->PE_Y_CNT = MMR_N_PE_Y;
	real_ptr->PE_CNT = real_ptr->PE_X_CNT * real_ptr->PE_Y_CNT;
	real_ptr->PE_SLOTS = MMR_MAX_LOCAL_TASKS;
	real_ptr->MC_SLOTS = real_ptr->PE_SLOTS * real_ptr->PE_CNT;

	return 0;
}

int sys_halt(tl_t *tl)
{
	/* Check if there are migrated tasks in the list and halt later */
	if(!(tm_empty() && psvc_empty())){
		puts("DEBUG: Not halting now due to task migration");
		return EAGAIN;
	}

	tcb_t *tcb = tcb_find(tl_get_task(tl));
	if(tcb != NULL && tcb_size() > 1){
		/* Check if error or if it has other management tasks */
		int ret = tcb_destroy_management(tcb);
		if(ret == EFAULT)
			puts("WARN: possible memory leak in non-destroyed TCB");
	}

	if(tcb != NULL){
		/* The app and sched list should be empty when there is no task running */
		app_derefer(tcb_get_app(tcb));
	}

	/* Disable scheduler */
	MMR_IRQ_MASK &= ~IRQ_SCHEDULER;
	
	/* Inform the mapper that this PE is ready to halt */
	int pe_halted[] = {PE_HALTED, MMR_NI_CONFIG};
	sys_kernel_writepipe(
		pe_halted, 
		sizeof(pe_halted), 
		tl_get_task(tl), 
		tl_get_addr(tl)
	);

	return 0;
}

int sys_end_simulation(tcb_t *tcb)
{
	if(tcb_get_id(tcb) >> 8 != 0)
		return -EACCES;

	MMR_END_SIM = 1;
	return 0;
}
