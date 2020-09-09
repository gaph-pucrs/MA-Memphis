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
unsigned int 	schedule_after_syscall;		
unsigned int 	cluster_master_address;		//!< Store the cluster master XY address
TCB 			idle_tcb;					
Message 		msg_write_pipe;				//!< Message variable which is used to copy a message and send it by the NoC


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
