/*!\file kernel_slave.c
 * MEMPHIS VERSION - 8.0 - support for RT applications
 *
 * Distribution:  June 2016
 *
 * Edited by: Marcelo Ruaro - contact: marcelo.ruaro@acad.pucrs.br
 *
 * Research group: GAPH-PUCRS   -  contact:  fernando.moraes@pucrs.br
 *
 * \brief
 * Kernel slave is the system slave used to execute user's tasks.
 *
 * \detailed
 * kernel_slave is the core of the OS running into the slave processors.
 * Its job is to runs the user's task. It communicates whit the kernel_master to receive new tasks
 * and also notifying its finish.
 * The kernel_slave file uses several modules that implement specific functions
 */

#include "kernel_slave.h"

#include "../../../include/kernel_pkg.h"
#include "../../include/api.h"
#include "../../include/plasma.h"
#include "../../include/services.h"
#include "../../modules/task_location.h"
#include "../../modules/packet.h"
#include "../../modules/communication.h"
#include "../../modules/pending_service.h"
#include "../../modules/task_scheduler.h"
#include "../../modules/utils.h"
#if MIGRATION_ENABLED
#include "../../modules/task_migration.h"
#endif

//Globals
unsigned int 	net_address;				//!< Store the current XY address
unsigned int 	schedule_after_syscall;		//!< Signals the syscall function (assembly implemented) to call the scheduler after the syscall
unsigned int 	cluster_master_address;		//!< Store the cluster master XY address
unsigned int 	last_idle_time;				//!< Store the last idle time duration
unsigned int 	total_slack_time;			//!< Store the total of the processor idle time
TCB 			idle_tcb;					//!< TCB pointer used to run idle task
TCB *			current;					//!< TCB pointer used to store the current task executing into processor
Message 		msg_write_pipe;				//!< Message variable which is used to copy a message and send it by the NoC


/** Assembles and sends a TASK_TERMINATED packet to the master kernel
 *  \param terminated_task Terminated task TCB pointer
 */
void send_task_terminated(TCB * terminated_task){

	ServiceHeader *p = get_service_header_slot();

	p->header = terminated_task->master_address;

	p->service = TASK_TERMINATED;

	p->task_ID = terminated_task->id;

	p->master_ID = cluster_master_address;

	send_packet(p, 0, 0);

	if (terminated_task->master_address != cluster_master_address){

		p = get_service_header_slot();

		p->header = cluster_master_address;

		p->service = TASK_TERMINATED_OTHER_CLUSTER;

		p->task_ID = terminated_task->id;

		p->master_ID = terminated_task->master_address;

		send_packet(p, 0, 0);
	}

}

/** Assembles and sends a TASK_ALLOCATED packet to the master kernel
 *  \param allocated_task Allocated task TCB pointer
 */
void send_task_allocated(TCB * allocated_task){

	ServiceHeader *p = get_service_header_slot();

	p->header = allocated_task->master_address;

	p->service = TASK_ALLOCATED;

	p->task_ID = allocated_task->id;

	p->master_ID = cluster_master_address;

	send_packet(p, 0, 0);
}

/** Assembles and sends a MESSAGE_DELIVERY packet to a consumer task located into a slave processor
 *  \param producer_task ID of the task that produce the message (Send())
 *  \param consumer_task ID of the task that consume the message (Receive())
 *  \param msg_ptr Message pointer
 */
void send_message_delivery(int producer_task, int consumer_task, int consumer_PE, Message * msg_ptr){

	ServiceHeader *p = get_service_header_slot();

	p->header = consumer_PE;

	p->service = MESSAGE_DELIVERY;

	p->producer_task = producer_task;

	p->consumer_task = consumer_task;

	p->msg_lenght = msg_ptr->length;

	while (msg_ptr->length > MSG_SIZE)
		puts("ERROR: message lenght higher than MSG_SIZE\n");

	send_packet(p, (unsigned int)msg_ptr->msg, msg_ptr->length);

}

/** Assembles and sends a REAL_TIME_CHANGE packet to the master kernel
 *  \param tcb_ptr TCB pointer of the task that change its real-time parameters
 */
void send_task_real_time_change(TCB * tcb_ptr){

	ServiceHeader * p = get_service_header_slot();

	p = get_service_header_slot();

	p->header = cluster_master_address;

	p->service = REAL_TIME_CHANGE;

	p->task_ID = tcb_ptr->id;

	p->utilization = tcb_ptr->scheduling_ptr->utilization;

	putsv("Send real time change, utilization: ", p->utilization);

	send_packet(p, 0, 0);
}

/** Assembles and sends a SLACK_TIME_REPORT packet to the master kernel. ATTENTION: currently
 * slack time report to manager is disabled because only disturbs the debuggability of the system
 */
void send_slack_time_report(){

	return; //Remove this line to restore the slack time report send to manager

	ServiceHeader * p = get_service_header_slot();

	p->header = cluster_master_address;

	p->service = SLACK_TIME_REPORT;

	p->cpu_slack_time = ( (total_slack_time*100) / SLACK_TIME_WINDOW);

	send_packet(p, 0, 0);
}

/** Assembles and sends a UPDATE_TASK_LOCATION packet to a slave processor. Useful because task migration
 * \param target_proc Target slave processor which the packet will be sent
 * \param task_id Task ID that have its location updated
 * \param new_task_location New location (slave processor address) of the task
 */
void send_update_task_location(unsigned int target_proc, unsigned int task_id, unsigned int new_task_location){

	ServiceHeader *p = get_service_header_slot();

	if (target_proc != net_address){

		p->header = target_proc;

		p->service = UPDATE_TASK_LOCATION;

		p->task_ID = task_id;

		p->allocated_processor = new_task_location;

		send_packet(p, 0, 0);
	}

	if (target_proc != new_task_location){

		p = get_service_header_slot();

		p->header = new_task_location;

		p->service = UPDATE_TASK_LOCATION;

		p->task_ID = task_id;

		p->allocated_processor = new_task_location;

		send_packet(p, 0, 0);
	}
}

/** Assembles and sends a DATA_AV packet to producer task into a slave processor
 * \param producer_task Producer task ID (Send())
 * \param consumer_task Consumer task ID (Receive())
 * \param targetPE Processor address of the consumer task
 * \param requestingPE Processor address of the producer task
 */
void send_data_av(int producer_task, int consumer_task, unsigned int targetPE, unsigned int requestingPE)
{
	ServiceHeader *p = get_service_header_slot();

	p->header = targetPE;

	p->service = DATA_AV;

	p->requesting_processor = requestingPE;

	p->producer_task = producer_task;

	p->consumer_task = consumer_task;

	send_packet(p, 0, 0);
}

/** Assembles and sends a MESSAGE_REQUEST packet to producer task into a slave processor
 * \param producer_task Producer task ID (Send())
 * \param consumer_task Consumer task ID (Receive())
 * \param targetPE Processor address of the producer task
 * \param requestingPE Processor address of the consumer task
 * \param insert_pipe_flag Tells to the producer PE that the message not need to be by passed again
 */
void send_message_request(int producer_task, int consumer_task, unsigned int targetPE, unsigned int requestingPE, int insert_pipe_flag){


	ServiceHeader *p = get_service_header_slot();

	p->header = targetPE;

	p->service = MESSAGE_REQUEST;

	p->requesting_processor = requestingPE;

	p->producer_task = producer_task;

	p->consumer_task = consumer_task;

	p->insert_request = insert_pipe_flag;

	send_packet(p, 0, 0);
}


/** Useful function to writes a message into the task page space
 * \param task_tcb_ptr TCB pointer of the task
 * \param msg_lenght Lenght of the message to be copied
 * \param msg_data Message data
 */
void write_local_msg_to_task(TCB * task_tcb_ptr, int msg_lenght, int * msg_data){

	Message * msg_ptr;

	msg_ptr = (Message*)((task_tcb_ptr->offset) | ((unsigned int)task_tcb_ptr->reg[3])); //reg[3] = address message

	msg_ptr->length = msg_lenght;

	for (int i=0; i<msg_ptr->length; i++)
		msg_ptr->msg[i] = msg_data[i];

	//Unlock the blocked task
	task_tcb_ptr->reg[0] = TRUE;

	//Release task to execute
	task_tcb_ptr->scheduling_ptr->waiting_msg = 0;
}

/** Syscall handler. It is called when a task calls a function defined into the api.h file
 * \param service Service of the syscall
 * \param arg0 Generic argument
 * \param arg1 Generic argument
 * \param arg2 Generic argument
 */
int Syscall(unsigned int service, unsigned int arg0, unsigned int arg1, unsigned int arg2) {

	Message *msg_read;
	Message *msg_write;
	PipeSlot *pipe_ptr;
	MessageRequest * msg_req_ptr;
	int consumer_task;
	int producer_task;
	int producer_PE;
	int consumer_PE;
	int appID;
	int ret;

	schedule_after_syscall = 0;

	switch (service) {

		case EXIT:

			schedule_after_syscall = 1;

			//Deadlock avoidance: avoids to send a packet when the DMNI is busy in send process
			//Also, due task migration sincronization messages, the producer task cannot finish it execution while have messages in PIPE
			if (MemoryRead(DMNI_SEND_ACTIVE) || search_PIPE_producer(current->id)){
				return 0;
			}

			puts("Task id: "); puts(itoa(current->id)); putsv(" terminated at ", MemoryRead(TICK_COUNTER));

			send_task_terminated(current);

			clear_scheduling(current->scheduling_ptr);

			appID = current->id >> 8;

			if ( !is_another_task_running(appID) ){

				clear_app_tasks_locations(appID);
			}

			return 1;

		case WRITEPIPE:

			producer_task =  current->id;
			consumer_task = (int) arg1;

			if(!arg2)	/* Synced write must send complete app|task id */
				consumer_task |= (producer_task & 0xF0);

			// puts("WRITEPIPE - prod: "); puts(itoa(producer_task)); putsv(" consumer ", consumer_task); putsv(" synced=", arg2);

			consumer_PE = get_task_location(consumer_task);

			/* Check if consumer task is allocated */
			if(consumer_PE == -1){
				// schedule_after_syscall = 1;
				
				/* @todo Discuss */
				if(!arg2){	/* Synced write can send to other apps and cannot wait for TASK_RELEASE */
					/* Task is blocked until a TASK_RELEASE packet is received */
					current->scheduling_ptr->status = BLOCKED;
				}
				return 0;
			}

			/* Points the message in the task page. Address composition: offset + msg address */
			msg_read = (Message *)((current->offset) | arg0);

			/* Test if the application passed a invalid message lenght */
			if(msg_read->length > MSG_SIZE || msg_read->length < 0){
				putsv("ERROR: Message lenght must be 0 or higher and lower than MSG_SIZE", msg_read->length);
				while(1);
			}

			/* Searches if there is a message request to the produced message */
			msg_req_ptr = remove_message_request(producer_task, consumer_task);

			if(msg_req_ptr){	/* Message request found! */

				if(msg_req_ptr->requester_proc == net_address){ /* Local consumer */

					/* Writes to the consumer page address */
					TCB * requesterTCB = searchTCB(consumer_task);

					write_local_msg_to_task(requesterTCB, msg_read->length, msg_read->msg);

				#if MIGRATION_ENABLED
					if(requesterTCB->proc_to_migrate != -1){
						migrate_dynamic_memory(requesterTCB);
						schedule_after_syscall = 1;
					}
				#endif

				} else {	/* Remote consumer */

					/* Send a MESSAGE_DELIVERY */

					/* Deadlock avoidance: avoid sending a packet when the DMNI is busy */
					if(MemoryRead(DMNI_SEND_ACTIVE)){
						/* Restore the message request */
						msg_req_ptr->requested = producer_task;
						msg_req_ptr->requester = consumer_task;
						return 0;
					}

					msg_write_pipe.length = msg_read->length;

					/* Copy to pipe to avoid message going out of scope */
					for(int i=0; i < msg_read->length; i++)
						msg_write_pipe.msg[i] = msg_read->msg[i];

					send_message_delivery(producer_task, consumer_task, msg_req_ptr->requester_proc, &msg_write_pipe);

				}
			} else { /* Message not requested yet */
				TCB *consumer_tcb = 0;
				if(consumer_PE == net_address)
					consumer_tcb = searchTCB(consumer_task);

				if(arg2 && consumer_tcb && consumer_tcb->scheduling_ptr->waiting_msg == WAITING_DATA_AV){	/* Local consumer waiting DATA_AV packet */
					/* Bypass and write directly to buffer */
					write_local_msg_to_task(consumer_tcb, msg_read->length, msg_read->msg);
					
				#if MIGRATION_ENABLED
					if(requesterTCB->proc_to_migrate != -1){
						migrate_dynamic_memory(requesterTCB);
						schedule_after_syscall = 1;
					}
				#endif
				} else if (get_PIPE_free_position){	/* Pipe has slot */
					if(arg2){
						if(consumer_PE == net_address){
							/* Insert a DATA_AV to consumer table */
							insert_data_av(producer_task, consumer_task, net_address);
						} else {
							/* Send DATA_AV to consumer PE */
							
							/* Deadlock avoidance: avoids to send a packet when the DMNI is busy in send process */
							if(MemoryRead(DMNI_SEND_ACTIVE))
								return 0;

							send_data_av(producer_task, consumer_task, consumer_PE, net_address);
						}
					}

					/* Store message in Pipe. Will be sent when a REQUEST is received */
					add_PIPE(producer_task, consumer_task, msg_read);
				} else {
					/* No PIPE space. Must retry */
					schedule_after_syscall = 1;
					return 0;
				}
			}

			return 1;

		case READPIPE:

			consumer_task =  current->id;
			producer_task = (int) arg1;

			appID = consumer_task >> 8;
			producer_task = (appID << 8) | producer_task;

			//puts("READPIPE - prod: "); puts(itoa(producer_task)); putsv(" consumer ", consumer_task);

			producer_PE = get_task_location(producer_task);

			//Test if the producer task is not allocated
			if (producer_PE == -1){
				//Task is blocked until its a TASK_RELEASE packet
				current->scheduling_ptr->status = BLOCKED;
				return 0;
			}

			if (producer_PE == net_address){ //Local producer

				//Searches if the message is in PIPE (local producer)
				pipe_ptr = remove_PIPE(producer_task, consumer_task);

				if (pipe_ptr == 0){

					//Stores the request into the message request table (local producer)
					insert_message_request(producer_task, consumer_task, net_address);

				} else {

					//Message was found in pipe, writes to the consumer page address (local producer)

					msg_write = (Message*) arg0;

					msg_write = (Message*)((current->offset) | ((unsigned int)msg_write));

					msg_write->length = pipe_ptr->message.length;

					for (int i = 0; i<msg_write->length; i++) {
						msg_write->msg[i] = pipe_ptr->message.msg[i];
					}

					return 1;
				}

			} else { //Remote producer : Sends the message request (remote producer)

				//Deadlock avoidance: avoids to send a packet when the DMNI is busy in send process
				if ( MemoryRead(DMNI_SEND_ACTIVE) )
					return 0;

				send_message_request(producer_task, consumer_task, producer_PE, net_address, 0);

			}

			//Sets task as waiting blocking its execution, it will execute again when the message is produced by a WRITEPIPE or incoming MSG_DELIVERY
			current->scheduling_ptr->waiting_msg = WAITING_DELIVERY;

			schedule_after_syscall = 1;

			return 0;

		case GETTICK:

			return MemoryRead(TICK_COUNTER);

		break;

		case ECHO:

			puts("$$$_");
			puts(itoa(net_address>>8));puts("x");puts(itoa(net_address&0xFF)); puts("_");
			puts(itoa(current->id >> 8)); puts("_");
			puts(itoa(current->id & 0xFF)); puts("_");
			puts((char *)((current->offset) | (unsigned int) arg0));
			puts("\n");

		break;

		case REALTIME:

			//Deadlock avoidance: avoids to send a packet when the DMNI is busy in send process
			if (MemoryRead(DMNI_SEND_ACTIVE)){
				return 0;
			}

			//putsv("\nReal-time to task: ", current->id);

			real_time_task(current->scheduling_ptr, arg0, arg1, arg2);

			//send_task_real_time_change(current);

			schedule_after_syscall = 1;

			return 1;

		break;

		case IOSEND:

			if ( MemoryRead(DMNI_SEND_ACTIVE) ){
				return 0;
			}

			producer_task =  current->id;
			consumer_task = (int) arg1; //In this case the consumer is not a task but a peripheral

			puts("Nothing to be done! System SendIO not implemented yet!\n");

			/*TODO: implement the protocol between the distributed usage of peripheral among applications
			 * Such protocol must ensure synchronization between requesting task to create a fair peripheral access
			 * Distributed mutual exclusion algorithm should be investigated.
			 */

			break;

		case IORECEIVE:

			if ( MemoryRead(DMNI_SEND_ACTIVE) ){
				return 0;
			}

			puts("Nothing to be done! System ReceiveIO not implemented yet!\n");

			/*TODO: implement the protocol between the distributed usage of peripheral among applications
			 * Such protocol must ensure synchronization between requesting task to create a fair peripheral access
			 * Distributed mutual exclusion algorithm should be investigated.
			 */

			break;
	}

	return 0;
}

/*--------------------------------------------------------------------------------------------------------
* printTaskInformations
*
* DESCRIPTION:
*    Prints (in hexadecimal) the code and data of task memory page
*--------------------------------------------------------------------------------------------------------*/
void printTaskInformations(TCB *task_tcb, char text, char bss_data, char stack){
	int i;
	unsigned int *offset;
	unsigned int stack_lenght;

	unsigned int stack_addr = task_tcb->reg[25];

	while((PAGE_SIZE - stack_addr) % 4) stack_addr--;
	stack_lenght = (PAGE_SIZE - stack_addr) / 4;

	puts("------------------------TASK INFORMATION----------------------------\n");
	puts("\nID: "); puts(itoa(task_tcb->id));

	puts("\n\nOffset: "); puts("\t"); puts(itoh(task_tcb->offset));

	puts("\n\nStack pointer: "); puts(itoh(task_tcb->reg[25]));

	puts("\n\nPC: "); puts(itoa(task_tcb->pc));

	puts("\n\nPAGESIZE: "); puts(itoa(PAGE_SIZE));

	puts("\nREGISTERS\n");
	for (i=0; i<30; i++){
		puts ("R"); puts(itoa(i)); puts(":\t"); puts(itoa(task_tcb->reg[i]));  puts("\n");
	}

	if (text){
		puts("\nTEXT\n");
		offset = task_tcb->offset;
		for(i=0; i<task_tcb->text_lenght; i++){
			puts(itoh(offset[i])); puts("\n");
		}
	}

	if (bss_data){
		puts("\nBSS E DATA\n");
		offset = (task_tcb->offset + (task_tcb->text_lenght*4));
		for(i=0; i<task_tcb->bss_lenght + task_tcb->data_lenght; i++){
			puts(itoh(offset[i])); puts("\n");
		}
	}

	if (stack) {
		puts("\nSTACK\n");
		offset = task_tcb->offset + task_tcb->reg[25];
		for(i=0; i<stack_lenght; i++){
			puts(itoh(offset[i])); puts("\n");
		}
	}

	puts("---------------------------------------------------------------------\n");

}

/** Handles a new packet from NoC
 */
int handle_packet(volatile ServiceHeader * p) {

	int need_scheduling, code_lenght, app_ID, task_loc;
	unsigned int app_tasks_location[MAX_TASKS_APP];
	PipeSlot * slot_ptr;
	Message * msg_ptr;
	TCB * tcb_ptr = 0;

	need_scheduling = 0;

	switch (p->service) {

	case MESSAGE_REQUEST: //This case is the most complicated of the Memphis if you understand it, so you understand all task communication protocol
	//This case sincronizes the communication messages also in case of task migration, the migration allows several scenarios, that are handled inside this case

		//Gets the location of the producer task
		task_loc = get_task_location(p->producer_task);

		//Test if the task was migrated to this processor but have message produced in the old processor
		//In this case is necessary to forward the message request to the old processor
		if (searchTCB(p->producer_task) && task_loc != net_address){
			if (p->insert_request)
				insert_message_request(p->producer_task, p->consumer_task, p->requesting_processor);
			else
				//MESSAGE_REQUEST by pass
				send_message_request(p->producer_task, p->consumer_task, task_loc, p->requesting_processor, 0);

			break;
		}

		//Remove msg from PIPE, if there is no message, them slot_ptr will be 0
		//Note that this line, is below to the by pass above, becase we need to avoid that the a message be removed if still there are other messages in pipe of the old proc
		slot_ptr = remove_PIPE(p->producer_task, p->consumer_task);

		//Test if there is no message in PIPE
		if (slot_ptr == 0){

			//Test if the producer task is running at this processor, this conditions work togheter to the first by pass condition (above).
			if (task_loc == net_address){

				insert_message_request(p->producer_task, p->consumer_task, p->requesting_processor);

			} else { //If not, the task was migrated and the requester message_request need to be forwarded to the correct producer proc

				//If there is no more messages in the pipe of the producer, update the location of the migrated task at the remote proc
				if ( search_PIPE_producer(p->producer_task) == 0)
					send_update_task_location(p->requesting_processor, p->producer_task, task_loc);

				//MESSAGE_REQUEST by pass with flag insert in pipe enabled (there is no more message for the requesting task in this processor)
				send_message_request(p->producer_task, p->consumer_task, task_loc, p->requesting_processor, 1);
			}

		//message found, send it!!
		} else if (p->requesting_processor != net_address){

			send_message_delivery(p->producer_task, p->consumer_task, p->requesting_processor, &slot_ptr->message);

		//This else is executed when this slave received a own MESSAGE_REQUEST due a task migration by pass
		} else {

			tcb_ptr = searchTCB(p->consumer_task);

			write_local_msg_to_task(tcb_ptr, slot_ptr->message.length, slot_ptr->message.msg);
		}

		break;

	case  MESSAGE_DELIVERY:

		tcb_ptr = searchTCB(p->consumer_task);

		msg_ptr = (Message *)(tcb_ptr->offset | tcb_ptr->reg[3]);

		msg_ptr->length = p->msg_lenght;

		DMNI_read_data((unsigned int)msg_ptr->msg, msg_ptr->length);

		tcb_ptr->reg[0] = 1;

		//Release task to execute
		tcb_ptr->scheduling_ptr->waiting_msg = 0;

#if MIGRATION_ENABLED
		if (tcb_ptr->proc_to_migrate != -1){

			migrate_dynamic_memory(tcb_ptr);

			need_scheduling = 1;

		} else
#endif

		if (current == &idle_tcb){
			need_scheduling = 1;
		}

		break;

	case TASK_ALLOCATION:

		tcb_ptr = search_free_TCB();

		tcb_ptr->pc = 0;

		tcb_ptr->id = p->task_ID;

		puts("Task id: "); puts(itoa(tcb_ptr->id)); putsv(" allocated at ", MemoryRead(TICK_COUNTER));

		code_lenght = p->code_size;

		tcb_ptr->text_lenght = code_lenght;

		tcb_ptr->master_address = p->master_ID;

		tcb_ptr->proc_to_migrate = -1;

		tcb_ptr->scheduling_ptr->remaining_exec_time = MAX_TIME_SLICE;

		DMNI_read_data(tcb_ptr->offset, code_lenght);

		tcb_ptr->scheduling_ptr->status = BLOCKED;

		send_task_allocated(tcb_ptr);

		if (current == &idle_tcb){
			need_scheduling = 1;
		}

		add_task_location(p->task_ID, net_address);

		putsv("Code lenght: ", code_lenght);
		putsv("Master ID: ", p->master_ID);

		//printTaskInformations(tcb_ptr, 1, 1, 0);

		break;

	case TASK_RELEASE:

		tcb_ptr = searchTCB(p->task_ID);

		app_ID = p->task_ID >> 8;

		//putsv("-> TASK RELEASE received to task ", p->task_ID);

		tcb_ptr->data_lenght = p->data_size;

		tcb_ptr->bss_lenght = p->bss_size;

		tcb_ptr->text_lenght = tcb_ptr->text_lenght - tcb_ptr->data_lenght;

		if (tcb_ptr->scheduling_ptr->status == BLOCKED)
			tcb_ptr->scheduling_ptr->status = READY;

		DMNI_read_data( (unsigned int) app_tasks_location, p->app_task_number);

		for (int i = 0; i < p->app_task_number; i++){
			add_task_location(app_ID << 8 | i, app_tasks_location[i]);
		}

		if (current == &idle_tcb){
			need_scheduling = 1;
		}

		break;

	case UPDATE_TASK_LOCATION:

		if (is_another_task_running(p->task_ID >> 8) ){

			remove_task_location(p->task_ID);

			add_task_location(p->task_ID, p->allocated_processor);

		}

		break;

	case INITIALIZE_SLAVE:

		cluster_master_address = p->source_PE;

		putsv("Slave initialized by cluster address: ", cluster_master_address);

		break;

#if MIGRATION_ENABLED
		case TASK_MIGRATION:
		case MIGRATION_CODE:
		case MIGRATION_TCB:
		case MIGRATION_TASK_LOCATION:
		case MIGRATION_MSG_REQUEST:
		case MIGRATION_STACK:
		case MIGRATION_DATA_BSS:

			need_scheduling = handle_migration(p, cluster_master_address);

			break;
#endif

	default:
		putsv("ERROR: service unknown: ", MemoryRead(TICK_COUNTER));
		break;
	}

	return need_scheduling;
}

/** Generic task scheduler call
 */
void Scheduler() {

	Scheduling * scheduled;
	unsigned int scheduler_call_time;

	scheduler_call_time = MemoryRead(TICK_COUNTER);

	MemoryWrite(SCHEDULING_REPORT, SCHEDULER);

	#if MIGRATION_ENABLED
		if (current->proc_to_migrate != -1 && current->scheduling_ptr->status == RUNNING && current->scheduling_ptr->waiting_msg == 0)
			migrate_dynamic_memory(current);
	#endif

	scheduled = LST(scheduler_call_time);

	if (scheduled){

		//This cast is an approach to reduce the scheduler call overhead
		current = (TCB *) scheduled->tcb_ptr;

		MemoryWrite(SCHEDULING_REPORT, current->id);

	} else {

		current = &idle_tcb;	// schedules the idle task

		last_idle_time = MemoryRead(TICK_COUNTER);

        MemoryWrite(SCHEDULING_REPORT, IDLE);
	}

	MemoryWrite(TIME_SLICE, get_time_slice() );

	OS_InterruptMaskSet(IRQ_SCHEDULER);

}

/** Function called by assembly (into interruption handler). Implements the routine to handle interruption in Memphis
 * This function must implement a important rule: it cannot send a packet when the DMNI is already send a packet.
 * The interruption triggers according to the DMNI status, and the if-else statements inside this function ensure this
 * behavior.
 * \param status Status of the interruption. Signal the interruption type
 */
void OS_InterruptServiceRoutine(unsigned int status) {

	MemoryWrite(SCHEDULING_REPORT, INTERRUPTION);

	volatile ServiceHeader p;
	ServiceHeader * next_service;
	unsigned call_scheduler;


	if (current == &idle_tcb){
		total_slack_time += MemoryRead(TICK_COUNTER) - last_idle_time;
	}

	call_scheduler = 0;

	//***** Check if interruption comes from NoC
	if ( status & IRQ_NOC ){

		read_packet((ServiceHeader *)&p);

		if (MemoryRead(DMNI_SEND_ACTIVE) && (p.service == MESSAGE_REQUEST || p.service == TASK_MIGRATION) ){

			add_pending_service((ServiceHeader *)&p);

		} else {

			call_scheduler = handle_packet(&p);
		}

	//**** Handles remaining packets
	} else if (status & IRQ_PENDING_SERVICE) {

		next_service = get_next_pending_service();
		if (next_service){
			call_scheduler = handle_packet(next_service);
		}
	//***** Send the monitoring packet
	} else if (status & IRQ_SLACK_TIME){
		send_slack_time_report();
		total_slack_time = 0;
		MemoryWrite(SLACK_TIME_MONITOR, 0);
	}


	if ( status & IRQ_SCHEDULER ){

		call_scheduler = 1;
	}


	if (call_scheduler){

		Scheduler();

	} else if (current == &idle_tcb){

		last_idle_time = MemoryRead(TICK_COUNTER);

		MemoryWrite(SCHEDULING_REPORT, IDLE);

	} else {
		MemoryWrite(SCHEDULING_REPORT, current->id);
	}

    /*runs the scheduled task*/
    ASM_RunScheduledTask(current);
}

/** Clear a interruption mask
 * \param Mask Interruption mask clear
 */
unsigned int OS_InterruptMaskClear(unsigned int Mask) {

    unsigned int mask;

    mask = MemoryRead(IRQ_MASK) & ~Mask;
    MemoryWrite(IRQ_MASK, mask);

    return mask;
}

/** Set a interruption mask
 * \param Mask Interruption mask set
 */
unsigned int OS_InterruptMaskSet(unsigned int Mask) {

    unsigned int mask;

    mask = MemoryRead(IRQ_MASK) | Mask;
    MemoryWrite(IRQ_MASK, mask);

    return mask;
}

/** Idle function
 */
void OS_Idle() {
	for (;;){
		MemoryWrite(CLOCK_HOLD, 1);
	}
}

int main(){

	ASM_SetInterruptEnable(FALSE);

	idle_tcb.pc = (unsigned int) &OS_Idle;
	idle_tcb.id = 0;
	idle_tcb.offset = 0;

	total_slack_time = 0;

	last_idle_time = MemoryRead(TICK_COUNTER);

	current = &idle_tcb;

	net_address = MemoryRead(NI_CONFIG);

	puts("Initializing PE: "); puts(itoh(net_address)); puts("\n");

	init_communication();

	init_service_header_slots();

	init_task_location();

	init_TCBs();

	/*disable interrupts*/
	OS_InterruptMaskClear(0xffffffff);

	/*enables timeslice counter and wrapper interrupts*/
	OS_InterruptMaskSet(IRQ_SCHEDULER | IRQ_NOC | IRQ_PENDING_SERVICE | IRQ_SLACK_TIME);

	/*runs the scheduled task*/
	ASM_RunScheduledTask(current);

	return 0;
}
