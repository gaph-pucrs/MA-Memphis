/**
 * MA-Memphis
 * @file task_migration.c
 *
 * @author Marcelo Ruaro (marcelo.ruaro@acad.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date June 2016
 * 
 * @brief Defines the task migration functions
 */

#include <stdio.h>

#include <memphis.h>

#include "task_migration.h"
#include "services.h"
#include "packet.h"
#include "task_location.h"
#include "broadcast.h"
#include "dmni.h"

typedef struct _tm_ring_t {
	migrated_task_t tasks[PKG_MAX_LOCAL_TASKS];
	int tail;
} tm_ring_t;

tm_ring_t migrated_tasks;

void tm_init()
{
	for(int i = 0; i < PKG_MAX_LOCAL_TASKS; i++){
		migrated_tasks.tasks[i].task = -1;
	}
	migrated_tasks.tail = 0;
}

void tm_add(int task, int addr)
{
	migrated_tasks.tasks[migrated_tasks.tail].task = task;
	migrated_tasks.tasks[migrated_tasks.tail].addr = addr;
	migrated_tasks.tail++;
	migrated_tasks.tail %= PKG_MAX_LOCAL_TASKS;
}

int tm_get_migrated_addr(int task)
{
	for(int i = 0; i < PKG_MAX_LOCAL_TASKS; i++){
		if(migrated_tasks.tasks[i].task == task)
			return migrated_tasks.tasks[i].addr;
	}
	return -1;
}

void tm_migrate(tcb_t *tcb)
{
	/* Get target address */
	int migrate_addr = tcb_get_migrate_addr(tcb);

	tm_add(tcb_get_id(tcb), migrate_addr);

	/* Update task location of tasks of the same app running locally */
	tl_update_local(tcb_get_id(tcb), migrate_addr);

	/* Send base TCB info */
	// puts("Sending migration TCB\n");
	tm_send_tcb(tcb, migrate_addr);
	/* Send message request array (only what is needed) */
	// puts("Sending migration message request\n");
	tm_send_mr(tcb, migrate_addr);
	/* Send data available fifo */
	// puts("Sending migration data available\n");
	tm_send_data_av(tcb, migrate_addr);
	
	/* Send pipe */
	// puts("Sending migration pipe\n");
	opipe_t *opipe = tcb_get_opipe(tcb);

	if(opipe != NULL){
		// puts("Will send pipe migration\n");

		opipe_migrate(opipe, migrate_addr, tcb_get_id(tcb));

		tcb_destroy_opipe(tcb);
	}

	/* Send data and BSS */
	// puts("Sending migration data and bss\n");
	tm_send_data_bss(tcb, migrate_addr);
	/* Send heap */
	// puts("Sending migration heap");
	tm_send_heap(tcb, migrate_addr);
	/* Send stack */
	// puts("Sending migration stack\n");
	tm_send_stack(tcb, migrate_addr);	
	/* Send task location array */
	// puts("Sending migration task location\n");
	tm_send_tl(tcb, migrate_addr);
	
	/* Code (.text) is in another function */
	printf(
		"Task id %d migrated at time %d to processor %x\n", 
		tcb_get_id(tcb), 
		MMR_TICK_COUNTER, 
		migrate_addr
	);

	sched_clear(tcb);
	tcb_clear(tcb);
}

void tm_send_code(tcb_t *tcb)
{
	packet_t *packet = pkt_slot_get();

	packet->header = tcb_get_migrate_addr(tcb);
	packet->service = MIGRATION_CODE;
	packet->task_ID = tcb_get_id(tcb);
	packet->code_size = tcb_get_code_length(tcb);
	packet->mapper_task = tcb->mapper_task;
	packet->mapper_address = tcb->mapper_address;

	dmni_send(
		packet, 
		(unsigned int*)tcb_get_offset(tcb), 
		tcb_get_code_length(tcb) >> 2
	);
}

void tm_send_tcb(tcb_t *tcb, int addr)
{
	/* Send TCB */
	packet_t *packet = pkt_slot_get();

	/* Task info */
	packet->header = addr;
	packet->service = MIGRATION_TCB;
	packet->task_ID = tcb_get_id(tcb);

	/* RT constraints */
	packet->period = sched_get_period(tcb);
	packet->deadline = sched_get_deadline(tcb);
	packet->waiting_msg = sched_get_waiting_msg(tcb);
	packet->execution_time = sched_get_exec_time(tcb);

	/* Registers */
	packet->program_counter = tcb_get_pc(tcb);

	dmni_send(packet, tcb->registers, HAL_MAX_REGISTERS);
}

void tm_send_tl(tcb_t *tcb, int addr)
{
	packet_t *packet = pkt_slot_get();

	packet->header = addr;
	packet->task_ID = tcb_get_id(tcb);
	packet->service = MIGRATION_TASK_LOCATION;
	packet->request_size = tl_get_len(tcb);

	dmni_send(packet, (unsigned int*)tl_get_ptr(tcb), packet->request_size);
}

void tm_send_mr(tcb_t *tcb, int addr)
{
	unsigned int mr_len = mr_defrag(tcb);

	if(mr_len){
		// puts("Will send message request");
		packet_t *packet = pkt_slot_get();

		packet->header = addr;
		packet->service = MIGRATION_MSG_REQUEST;
		packet->task_ID = tcb_get_id(tcb);
		packet->request_size = mr_len;

		dmni_send(
			packet, 
			(unsigned int*)tcb_get_mr(tcb), 
			(mr_len*sizeof(message_request_t)) >> 2
		);
	}
}

void tm_send_data_av(tcb_t *tcb, int addr)
{
	unsigned int data_av_len = data_av_get_len_head_end(tcb);

	if(data_av_len){
		// puts("will send data_av migrate part 1\n");
		packet_t *packet = pkt_slot_get();

		packet->header = addr;
		packet->task_ID = tcb_get_id(tcb);
		packet->service = MIGRATION_DATA_AV;
		packet->request_size = data_av_len;

		dmni_send(
			packet, 
			(unsigned int*)data_av_get_buffer_head(tcb), 
			(data_av_len*sizeof(data_av_t)) >> 2
		);
	}

	data_av_len = data_av_get_len_start_tail(tcb);

	if(data_av_len){
		// puts("will send data_av migrate part 2\n");
		packet_t *packet = pkt_slot_get();

		packet->header = addr;
		packet->task_ID = tcb_get_id(tcb);
		packet->service = MIGRATION_DATA_AV;
		packet->request_size = data_av_len;

		dmni_send(
			packet, 
			(unsigned int*)data_av_get_buffer_start(tcb), 
			(data_av_len*sizeof(data_av_t)) >> 2
		);
	}
}

void tm_send_stack(tcb_t *tcb, int addr)
{
	/* Get the stack pointer */
	unsigned int stack_len = PKG_PAGE_SIZE - tcb_get_sp(tcb);

	/* Align to 32 bits */
	while(stack_len % 4)
		stack_len++;

	if(stack_len){
		packet_t *packet = pkt_slot_get();

		packet->header = addr;
		packet->service = MIGRATION_STACK;
		packet->task_ID = tcb_get_id(tcb);
		packet->stack_size = stack_len;

		dmni_send(
			packet, 
			(unsigned int*)(tcb_get_offset(tcb) + PKG_PAGE_SIZE - stack_len), 
			stack_len >> 2
		);
	}
}

void tm_send_heap(tcb_t *tcb, int addr)
{
	/* Get the stack pointer */
	unsigned int heap_start = 
		tcb_get_code_length(tcb) + 
		tcb_get_data_length(tcb) + 
		tcb_get_bss_length(tcb);
	unsigned int heap_len = tcb_get_heap_end(tcb) - heap_start;

	/* Align to 32 bits */
	while(heap_len % 4)
		heap_len++;

	if(heap_len){
		packet_t *packet = pkt_slot_get();

		packet->header = addr;
		packet->service = MIGRATION_HEAP;
		packet->task_ID = tcb_get_id(tcb);
		packet->heap_size = heap_len;

		dmni_send(
			packet, 
			(unsigned int*)(tcb_get_offset(tcb) + heap_start), 
			heap_len >> 2
		);
	}
}

void tm_send_data_bss(tcb_t *tcb, int addr)
{
	unsigned data_size = tcb_get_data_length(tcb);
	unsigned bss_size = tcb_get_bss_length(tcb);

	if(data_size + bss_size){
		packet_t *packet = pkt_slot_get();

		packet->header = addr;
		packet->service = MIGRATION_DATA_BSS;
		packet->task_ID = tcb_get_id(tcb);
		packet->data_size = data_size;
		packet->bss_size = bss_size;

		dmni_send(
			packet, 
			(unsigned int*)(tcb_get_offset(tcb) + tcb_get_code_length(tcb)), 
			(data_size + bss_size) >> 2
		);
	}
}

void tm_abort_task(int id, int addr)
{
	br_packet_t packet;

	packet.service = ABORT_TASK;
	packet.src_id = -1;
	packet.payload = id;
	while(!br_send(&packet, addr, BR_SVC_TGT));
}
