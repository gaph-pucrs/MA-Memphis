/*!\file task_migration.c
 * MEMPHIS VERSION - 8.0 - support for RT applications
 *
 * Distribution:  June 2016
 *
 * Created by: Marcelo Ruaro - contact: marcelo.ruaro@acad.pucrs.br
 *
 * Research group: GAPH-PUCRS   -  contact:  fernando.moraes@pucrs.br
 *
 * \brief
 * This module implements function relative to task migration. This module is used by slave kernel
 *
 */
#include "task_migration.h"

#include "../../include/kernel_pkg.h"
#include "../include/services.h"
#include "../include/plasma.h"
#include "task_location.h"
#include "task_scheduler.h"
#include "communication.h"
#include "utils.h"

#define TASK_MIGRATION_DEBUG	0		//!<When enable shows puts related to task migration

/**Assembles and sends a TASK_MIGRATED packet to the master kernel
 * \param migrated_task Migrated task ID
 * \param old_proc Old processor address of task
 * \param master_address Master address of the task
 */
void send_task_migrated(int migrated_task, int old_proc, unsigned int master_address){

	ServiceHeader * p = get_service_header_slot();

	p->header = master_address;

	p->service = TASK_MIGRATED;

	p->task_ID = migrated_task;

	p->released_proc = old_proc;

	send_packet(p, 0, 0);
}

/**This function os the core of task migration.
 * It is called by the source processor (the older processor)
 * Its job is to migrate to the new processor the dynamic data section that can change during task execution.
 * \param tcb_aux The TCB pointer of the task to be migrated
 */
void migrate_dynamic_memory(TCB * tcb_aux){

	unsigned int _stack_pointer;
	unsigned int stack_lenght;
	unsigned int processor;
	volatile unsigned int tcb_registers[30];
	volatile unsigned int task_location_array[MAX_TASKS_APP];
	unsigned int request_msg[REQUEST_SIZE*3];
	unsigned int app_id;
	unsigned int request_array_size;
	ServiceHeader * p;

#if TASK_MIGRATION_DEBUG
	putsv("\tmigrate dynamic memory to task: ", tcb_aux->id);
	puts("----> computing stack pointer size ....\n");
#endif

	processor = tcb_aux->proc_to_migrate;

	tcb_aux->proc_to_migrate = -1;

	_stack_pointer = tcb_aux->reg[25]; //plus one because the stack is initialized PAGE_SIZE - 1

#if TASK_MIGRATION_DEBUG
	putsv("\tstack pointer antes: ", _stack_pointer);
#endif

	while((PAGE_SIZE - _stack_pointer) % 4)
		_stack_pointer--;

#if TASK_MIGRATION_DEBUG
	putsv("\tstack pointer depois: ", _stack_pointer);
#endif

	stack_lenght = (PAGE_SIZE - _stack_pointer) / 4;

	_stack_pointer = tcb_aux->reg[25];

#if TASK_MIGRATION_DEBUG
	putsv("\tstack lenght: ", stack_lenght);
	putsv("\tstack address: ", _stack_pointer);
	puts("end computing stack pointer\n----> migrating TCB....\n");
#endif

	//------ tcb ------
	//puts("\n--- tcb ----\n");
	p = get_service_header_slot();

	p->header = processor;

	p->service = MIGRATION_TCB;

	p->task_ID = tcb_aux->id;

	//Send RT constraints: period
	p->period = tcb_aux->scheduling_ptr->period;
	//Send RT constraints: deadline
	p->deadline = tcb_aux->scheduling_ptr->deadline;
	//Send RT constraints: execution_time
	p->execution_time = tcb_aux->scheduling_ptr->execution_time;

	p->program_counter = tcb_aux->pc - tcb_aux->offset;

	for (int i=0; i<30; i++){
		if (i == 27)
			tcb_registers[i] = tcb_aux->reg[i] - tcb_aux->offset;
		else
			tcb_registers[i] = tcb_aux->reg[i];
	}

	send_packet(p, (unsigned int) &tcb_registers, 30);
	// ------- end tcb ------

#if TASK_MIGRATION_DEBUG
	puts("TCB migrated\n----> migrating task location....\n");
#endif

	// ----- task location ------
	p = get_service_header_slot();

	p->header = processor;

	p->task_ID = tcb_aux->id;

	p->service = MIGRATION_TASK_LOCATION;

	app_id = tcb_aux->id & 0xFF00;

	//Update task location
	remove_task_location(tcb_aux->id);
	add_task_location(tcb_aux->id, processor);

	for( int i=0; i<MAX_TASKS_APP; i++ ){

		if ( search_PIPE_producer(app_id | i) ) {
			task_location_array[i] = MemoryRead(NI_CONFIG); //net_address
		} else {
			task_location_array[i] = get_task_location( (app_id | i) );
		}
#if TASK_MIGRATION_DEBUG
		puts("Location task "); puts(itoa(app_id | i)); putsv(" : ", task_location_array[i]);
#endif
	}

	send_packet(p, (unsigned int) &task_location_array, MAX_TASKS_APP);
	// ----- end task location ------

#if TASK_MIGRATION_DEBUG
	puts("task location migrated\n----> migrating task request....\n");
#endif

	// ----- task request -----
	request_array_size = remove_all_requested_msgs(tcb_aux->id, request_msg);

	if (request_array_size > 0){

		p = get_service_header_slot();

		p->header = processor;

		p->service = MIGRATION_MSG_REQUEST;

		p->task_ID = tcb_aux->id;

		p->request_size = request_array_size;

		send_packet(p, (unsigned int) &request_msg, request_array_size);

#if TASK_MIGRATION_DEBUG
		for(int i=0; i<request_array_size; i++){
			putsvsv("request[", i,"]: ", request_msg[i]);
		}
#endif

	}
	// ----- end task request -----

#if TASK_MIGRATION_DEBUG
	puts("request migrated\n----> migrating stack....\n");
#endif

	// ----- stack -----
	p = get_service_header_slot();

	p->header = processor;

	p->service = MIGRATION_STACK;

	p->task_ID = tcb_aux->id;

	p->stack_size = stack_lenght;

	send_packet(p, (tcb_aux->offset + _stack_pointer), stack_lenght);
	// ----- end stack -----

#if TASK_MIGRATION_DEBUG
	puts("stack migrated\n----> migrating DATA and BSS....\n");
	putsvsv("DATA: ",tcb_aux->data_lenght, " BSS: ",tcb_aux->bss_lenght);
#endif

	// ------ data and bss -----
	p = get_service_header_slot();

	p->header = processor;

	p->service = MIGRATION_DATA_BSS;

	p->task_ID = tcb_aux->id;

	p->data_size = tcb_aux->data_lenght;

	p->bss_size = tcb_aux->bss_lenght;

	send_packet(p, tcb_aux->offset + (tcb_aux->text_lenght * 4) ,  (tcb_aux->data_lenght + tcb_aux->bss_lenght) );
	// ----- end data and bss -----

#if TASK_MIGRATION_DEBUG
	puts("migration FINISH, clearing task structures....\n");
	puts("finished\nTask id: "); puts(itoa(tcb_aux->id)); puts(" migrated to proc "); puts(itoa(processor)); putsv(" at ", MemoryRead(TICK_COUNTER));
#endif
	puts("Task id: "); puts(itoa(tcb_aux->id)); puts(" migrated at time "); puts(itoa(MemoryRead(TICK_COUNTER))); puts(" to processor "); puts(itoh(processor)); puts("\n");

	clear_scheduling(tcb_aux->scheduling_ptr);
	tcb_aux->pc = 0;
	tcb_aux->id = -1;
	tcb_aux->proc_to_migrate = -1;

}

/**Migrate the code data to the new processor.
 * It is called by the source processor (the older processor)
 * The code is static because corresponds the the task instructions loaded
 * when the task is allocated in a given processor
 * \param tcb_migration The TCB pointer of the task to be migrated
 */
void migrate_CODE(TCB* tcb_migration){

	ServiceHeader * p = get_service_header_slot();

	p->header = tcb_migration->proc_to_migrate;

	p->service = MIGRATION_CODE;

	p->master_ID = tcb_migration->master_address;

	p->task_ID = tcb_migration->id;

	p->code_size = tcb_migration->text_lenght;

	send_packet(p, tcb_migration->offset, tcb_migration->text_lenght);

#if TASK_MIGRATION_DEBUG
	putsv("\tCODE migrated with size ", p->code_size)
#endif
}

/**Handle the migration code, coping the code to a free page.
 * It is called by the target processor (the new processor)
 * \param p ServiceHeader pointer of the packet with the task code
 * \param migrate_tcb The TCB pointer of the task to be migrated
 */
void handle_migration_code(volatile ServiceHeader * p, TCB * migrate_tcb){

	migrate_tcb->scheduling_ptr->status = MIGRATING;

	migrate_tcb->id = p->task_ID;

	migrate_tcb->master_address = p->master_ID;

	migrate_tcb->text_lenght = p->code_size;

	DMNI_read_data(migrate_tcb->offset, migrate_tcb->text_lenght);
	//printTaskInformations(allocatingTCB, 1, 0, 0);

#if TASK_MIGRATION_DEBUG
	putsvsv("\tCODE received with size ", p->code_size, " the task offset is: ", migrate_tcb->offset);
#endif

}

/**Handles the migration of the task TCB information
 * It is called by the target processor (the new processor)
 * \param p ServiceHeader pointer of the packet with the TCB data
 * \param migrate_tcb The TCB pointer of the task to be migrated
 */
void handle_migration_TCB(volatile ServiceHeader * p, TCB * migrate_tcb){

	volatile unsigned int tcb_registers[30];

	migrate_tcb->pc = p->program_counter + migrate_tcb->offset;


	DMNI_read_data((unsigned int) &tcb_registers, 30);

	for (int i=0; i<30; i++){
		if (i == 27)
			migrate_tcb->reg[i] = tcb_registers[i] + migrate_tcb->offset;
		else
			migrate_tcb->reg[i] = tcb_registers[i];
	}

	if (p->period > 0) //This means that the task have RT parameters
		real_time_task(migrate_tcb->scheduling_ptr, p->period, p->deadline, p->execution_time);

#if TASK_MIGRATION_DEBUG
	puts("\tTCB received\n");
#endif
}

/**Handles the migration of the task task location data
 * It is called by the target processor (the new processor)
 * \param p ServiceHeader pointer of the packet with the task location data
 * \param migrate_tcb The TCB pointer of the task to be migrated
 */
void handle_migration_task_location(volatile ServiceHeader * p, TCB * migrate_tcb){

	volatile unsigned int task_location_array[MAX_TASKS_APP];
	unsigned int location;
	unsigned int app_id;
	unsigned int task_id;

	DMNI_read_data( (unsigned int) &task_location_array, MAX_TASKS_APP);

	app_id = migrate_tcb->id & 0xFF00;

#if TASK_MIGRATION_DEBUG
	puts("\treceiveing task location....\n");
#endif

	for( int i=0; i<MAX_TASKS_APP; i++ ){

		location = task_location_array[i];

		task_id = (app_id | i);

		#if TASK_MIGRATION_DEBUG
		puts("Location task "); puts(itoa(task_id)); putsv(" : ", location);
		#endif

		if ( location != -1 && get_task_location(task_id) == -1){

#if TASK_MIGRATION_DEBUG
			puts("updated\n");
#endif

			add_task_location(task_id, location);
		}

		/*if (task_location[app_id][i] == NOT_ALLOCATED && location != NOT_ALLOCATED){
			task_location[app_id][i] = location;
		}*/
	}

#if TASK_MIGRATION_DEBUG
	puts("\ttask location received\n");
#endif

}

/**Handles the migration of the task message request data
 * It is called by the target processor (the new processor)
 * \param p ServiceHeader pointer of the packet with the message request data
 * \param migrate_tcb The TCB pointer of the task to be migrated
 */
void handle_migration_request_msg(volatile ServiceHeader * p, TCB * migrate_tcb){

	volatile unsigned int request_msg[p->request_size];
	int requested, requester, requester_proc;
	unsigned int request_index, iterations;

	DMNI_read_data( (unsigned int) &request_msg, p->request_size);

	request_index = 0;

	iterations = p->request_size / 3;

#if TASK_MIGRATION_DEBUG
	puts("\treceiveing task request....\n");
	putsv("number of request: ", iterations);
#endif

	for( int i=0; i<iterations; i++ ) {
		requester = request_msg[request_index++];
		requested = request_msg[request_index++];
		requester_proc = request_msg[request_index++];

		if ( requested == migrate_tcb->id) {
#if TASK_MIGRATION_DEBUG
			putsvsv("requester: ", requester, " requested: ", requested);
#endif
			insert_message_request(requested, requester, requester_proc);
		}
	}

#if TASK_MIGRATION_DEBUG
	puts("\ttask request received\n");
#endif

}

/**Handles the migration of the task stack data
 * It is called by the target processor (the new processor)
 * \param p ServiceHeader pointer of the packet with the task stack data
 * \param migrate_tcb The TCB pointer of the task to be migrated
 */
void handle_migration_stack(volatile ServiceHeader * p, TCB * migrate_tcb){

	if (p->stack_size > 0){
		DMNI_read_data(migrate_tcb->offset + migrate_tcb->reg[25], p->stack_size);
	}

#if TASK_MIGRATION_DEBUG
	puts("\tSTACK received\n");
#endif
}

/**Handles the migration of the task DATA and BSS data sections
 * It is called by the target processor (the new processor)
 * \param p ServiceHeader pointer of the packet with the task DATA and BSS data sections
 * \param migrate_tcb The TCB pointer of the task to be migrated
 */
void handle_migration_DATA_BSS(volatile ServiceHeader * p, TCB * migrate_tcb, unsigned int master_address){

	migrate_tcb->data_lenght = p->data_size;

	migrate_tcb->bss_lenght = p->bss_size;

	if ((migrate_tcb->bss_lenght + migrate_tcb->data_lenght) > 0){

		DMNI_read_data(migrate_tcb->offset + (migrate_tcb->text_lenght*4), (migrate_tcb->bss_lenght + migrate_tcb->data_lenght));
	}

	migrate_tcb->scheduling_ptr->status = READY;

	migrate_tcb->proc_to_migrate = -1;

	migrate_tcb->scheduling_ptr->remaining_exec_time = MAX_TIME_SLICE;

	send_task_migrated(migrate_tcb->id, p->source_PE, master_address);

	//printTaskInformations(migrate_tcb, 1, 1, 1);

#if TASK_MIGRATION_DEBUG
	puts("\tDATA and BSS received\nMigration FINISH - task READY TO EXECUTE\n");
#endif
	puts("Task id: "); puts(itoa(migrate_tcb->id)); puts(" allocated by task migration at time "); puts(itoa(MemoryRead(TICK_COUNTER))); puts(" from processor "); puts(itoh(p->source_PE)); puts("\n");

}

/**Handles a task migration order from the kernel master
 * This function is called by the source processor (the old processor)
 * \param p ServiceHeader pointer of the packet with task migration order
 * \param tcb_ptr TCB pointer of the task to be migrated
 */
int handle_task_migration(volatile ServiceHeader * p, TCB * tcb_ptr){


#if TASK_MIGRATION_DEBUG
	putsvsv("##### \tTask migration order task id: ", p->task_ID, " to proc: ", p->allocated_processor);
#endif

	if (tcb_ptr && tcb_ptr->proc_to_migrate == -1){

		tcb_ptr->proc_to_migrate = p->allocated_processor;

		migrate_CODE(tcb_ptr);

#if TASK_MIGRATION_DEBUG
		putsv("\ttask status: ", tcb_ptr->status);
#endif

		if (tcb_ptr->scheduling_ptr->waiting_msg == 0){

#if TASK_MIGRATION_DEBUG
			puts("\tMigrou de primeira\n");
#endif

			migrate_dynamic_memory(tcb_ptr);

			return 1;
		}
	} else {
		puts ("ERROR: task not found or proc_to_migrated already assigned\n");
		while(1);
	}

	return 0;

}

/**Handles all task migration packets, calling the appropriated sub-function.
 * \param p ServiceHeader pointer of the packet a generic task migration packet
 * \param master_address Address of the kernel master of the slave processor
 * \return The necessity of call the scheduler, 1 - need scheduler, 0 not need scheduler
 */
int handle_migration(volatile ServiceHeader * p, unsigned int master_address){

	int need_scheduling = 0;

	TCB * migrate_tcb;

	if (p->service == MIGRATION_CODE){
		migrate_tcb = search_free_TCB();
	} else {
		migrate_tcb = searchTCB(p->task_ID);
	}

	switch(p->service){

		case TASK_MIGRATION:

			need_scheduling = handle_task_migration(p, migrate_tcb);

			break;

		case MIGRATION_CODE:

			handle_migration_code(p, migrate_tcb);

			break;

		case MIGRATION_TCB:

			handle_migration_TCB(p, migrate_tcb);

			break;

		case MIGRATION_TASK_LOCATION:

			handle_migration_task_location(p, migrate_tcb);

			break;

		case MIGRATION_MSG_REQUEST:

			handle_migration_request_msg(p, migrate_tcb);

			break;

		case MIGRATION_STACK:

			handle_migration_stack(p, migrate_tcb);

			break;

		case MIGRATION_DATA_BSS:

			handle_migration_DATA_BSS(p, migrate_tcb, master_address);

			need_scheduling = 1;

			break;
		default:
			puts("ERROR - Task Migration service unknown\n");
			break;
		}

	return need_scheduling;

}
