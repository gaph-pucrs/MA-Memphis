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

#include "syscall.h"
#include "services.h"
#include "task_location.h"
#include "task_migration.h"
#include "utils.h"

bool schedule_after_syscall;	//!< Signals the HAL syscall to call scheduler

int os_syscall(unsigned int service, unsigned int a1, unsigned int a2, unsigned int a3)
{
	schedule_after_syscall = false;
	switch(service){
		case EXIT:
			return os_exit();
		case WRITEPIPE:
			return os_writepipe(a1, a2, a3);
		case READPIPE:
			return os_readpipe(a1, a2, a3);
		case GETTICK:
			return os_get_tick();
		case APPID:
			return os_get_appid();
		case ECHO:
			return os_echo(a1);
		case REALTIME:
			return os_realtime(a1, a2, a3);
		default:
			puts("ERROR: Unknown service\n");
			return 0;
	}
}

bool os_exit()
{
	schedule_after_syscall = true;

	tcb_t *current = sched_get_current();

	/* Avoid sending a packet while DMNI is busy */
	/* Don't erase task with message in pipe */
	if(HAL_DMNI_SEND_ACTIVE || pipe_is_full(current))
		return false;

	/* Send TASK_TERMINATED */
	/** @todo Should be wrapped in a DATA_AV */
	tl_send_terminated(current);

	puts("Task id: "); puts(itoa(current->id)); putsv(" terminated at ", HAL_TICK_COUNTER);

	sched_clear(current);

	tcb_clear(current);

	return true;
}

bool os_writepipe(unsigned int msg_ptr, int cons_task, bool sync)
{
	tcb_t *current = sched_get_current();

	if(!sync)	/* Synced write must send complete app|task id */
		cons_task |= (current->id & 0xFF00);


	int cons_addr = tl_search(current, cons_task);

	/* Check if consumer task is allocated */
	if(cons_addr == -1){		
		/** @todo Discuss */
		if(!sync){	/* Synced write can send to other apps and cannot wait for TASK_RELEASE */
			/* Task is blocked until a TASK_RELEASE packet is received */
			sched_block(current);
			schedule_after_syscall = 1;
		}
		return false;
	}

	/* Points the message in the task page. Address composition: offset + msg address */
	message_t *message = (message_t*)(tcb_get_offset(current) | msg_ptr);

	/* Test if the application passed a invalid message lenght */
	if(message->length > MSG_SIZE || message->length < 0){
		putsv("ERROR: Message lenght must be 0 or higher and lower than MSG_SIZE", message->length);
		while(1);
	}

	/* Searches if there is a message request to the produced message */
	message_request_t *request = mr_peek(current, cons_task);

	if(request){	/* Message request found! */
		if(request->requester_address == HAL_NI_CONFIG){ /* Local consumer */
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
		} else {	/* Remote consumer */

			/* Send a MESSAGE_DELIVERY */

			/* Deadlock avoidance: avoid sending a packet when the DMNI is busy */
			/* Also, don't send a message if the previous is still in pipe */
			if(HAL_DMNI_SEND_ACTIVE || pipe_is_full(current))
				return false;

			/* Insert the message in the pipe to avoid overwrite by task */
			pipe_push(current, message, cons_task);

			/* Send through NoC */
			pipe_send(sched_get_current_id(), cons_task, request->requester_address, pipe_pop(current, cons_task));
			
			/* Remove the message request from buffer */
			mr_pop(request);
		}

	} else if(!pipe_is_full(current) && !HAL_DMNI_SEND_ACTIVE){	/* Pipe is free */
		if(sync){
			int prod_task = sched_get_current_id();
			if(cons_addr == HAL_NI_CONFIG){
				/* Insert a DATA_AV to consumer table */
				tcb_t *cons_tcb = tcb_search(cons_task);

				/* Insert DATA_AV to the consumer TCB */
				data_av_insert(cons_tcb, prod_task, HAL_NI_CONFIG);
				
				/* If consumer waiting for a DATA_AV, release the task */
				if(sched_is_waiting_data_av(cons_tcb))
					sched_release_wait(cons_tcb);

			} else {
				/* Send DATA_AV to consumer PE */
				data_av_send(cons_task, prod_task, cons_addr, HAL_NI_CONFIG);
			}
		}

		/* Store message in Pipe. Will be sent when a REQUEST is received */
		pipe_push(current, message, cons_task);

	} else {
		/* DMNI busy sending message or pipe full */
		if(pipe_is_full(current)){
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

		/* Test if the producer task is not allocated */
		if(prod_addr == -1){
			/* Task is blocked until its a TASK_RELEASE packet */
			sched_block(current);

			return false;
		}
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
	}

	if(prod_addr == HAL_NI_CONFIG){	/* Local producer */

		/* Get the producer TCB */
		tcb_t *prod_tcb = tcb_search(prod_task);

		/* Searches if the message is in PIPE (local producer) */
		pipe_t *pipe = pipe_pop(prod_tcb, cons_task);

		if(!pipe){
			/* Stores the request into the message request table */
			mr_insert(prod_tcb, cons_task, HAL_NI_CONFIG);

		} else {
			/* Message was found in pipe, writes to the consumer page address (local producer) */
			message_t *message = (message_t*)(tcb_get_offset(current) | msg_ptr);
			message_t *msg_src = pipe_get_message(pipe);

			pipe_transfer(msg_src, message);

			if(sched_is_waiting_request(prod_tcb))
				sched_release_wait(prod_tcb);

			return true;
		}

	} else { /* Remote producer : Sends the message request */

		/* Deadlock avoidance: avoids to send a packet when the DMNI is busy in send process */
		if(HAL_DMNI_SEND_ACTIVE)
			return false;
		
		if(sync){
			/* DATA_AV is processed, erase it */
			data_av_pop(current);
		}

		/* Send the message request through NoC */
		mr_send(prod_task, cons_task, prod_addr, HAL_NI_CONFIG);
	}

	/* Sets task as waiting blocking its execution, it will execute again when the message is produced by a WRITEPIPE or incoming MSG_DELIVERY */
	sched_set_wait_delivery(current);
	schedule_after_syscall = 1;

	return true;
}

unsigned int os_get_tick()
{
	return HAL_TICK_COUNTER;
}

int os_get_appid()
{
	tcb_t *current = sched_get_current();
	return tcb_get_appid(current);
}

bool os_echo(unsigned int msg_ptr)
{
	/** 
	 * @todo Add a printf function that calls the os_echo and a "backend" for 
	 * OS messages
	 */
	tcb_t *current = sched_get_current();
	int id = sched_get_current_id();
	puts("$$$_");
	puts(itoa(HAL_NI_CONFIG >> 8));
	puts("x");
	puts(itoa(HAL_NI_CONFIG & 0xFF));
	puts("_");
	puts(itoa(id >> 8));
	puts("_");
	puts(itoa(id & 0xFF));
	puts("_");
	puts((char*)(tcb_get_offset(current) | msg_ptr));
	puts("\n");

	return true;
}

bool os_realtime(unsigned int period, int deadline, unsigned int exec_time)
{
	tcb_t *current = sched_get_current();
	sched_real_time_task(current, period, deadline, exec_time);

	schedule_after_syscall = 1;

	return true;
}
