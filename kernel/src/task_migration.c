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

#include "task_migration.h"

#include <stdlib.h>
#include <stdio.h>

#include <memphis.h>
#include <memphis/services.h>

#include "broadcast.h"
#include "dmni.h"
#include "mmr.h"

list_t _tms;

void tm_init()
{
	list_init(&_tms);
}

tl_t *tm_find(int task)
{
	return tl_find(&_tms, task);
}

tl_t *tm_emplace_back(int task, int addr)
{
	tl_t *tl = malloc(sizeof(tl_t));

	if(tl == NULL)
		return NULL;

	tl_set(tl, task, addr);

	list_entry_t *entry = list_push_back(&_tms, tl);

	if(entry == NULL){
		free(tl);
		return NULL;
	}

	return tl;
}

void tm_migrate(tcb_t *tcb)
{
	/* Get target address */
	int addr = tcb_get_migrate_addr(tcb);
	int id = tcb_get_id(tcb);

	tl_t *tm = tm_emplace_back(id, addr);
	if(tm == NULL){
		puts("WARNING: not enough memory, not migrating.");
		return;
	}

	/* Send base TCB info */
	// puts("Sending migration TCB");
	tm_send_tcb(tcb, id, addr);
	/* Send task location array */
	// puts("Sending migration app (task location)");
	app_t *app = tcb_get_app(tcb);
	tm_send_app(tcb, app, id, addr);
	/* Send data available fifo */
	// puts("Sending migration task location (DATA_AV)");
	tm_send_tl(tcb, tcb_get_davs(tcb), MIGRATION_DATA_AV, id, addr);
	/* Send message request array (only what is needed) */
	// puts("Sending migration task location (MESSAGE_REQUEST)");
	tm_send_tl(tcb, tcb_get_msgreqs(tcb), MIGRATION_MSG_REQUEST, id, addr);
	/* Send pipe */
	// puts("Sending migration pipe");
	tm_send_opipe(tcb, id, addr);
	/* Send data and BSS */
	// puts("Sending migration data and bss");
	tm_send_data_bss(tcb, id, addr);
	/* Send heap */
	// puts("Sending migration heap");
	tm_send_heap(tcb, id, addr);
	/* Send stack */
	// puts("Sending migration stack");
	tm_send_stack(tcb, id, addr);
	/* Send scheduler data */
	// puts("Sending migration scheduler");
	tm_send_sched(tcb, id, addr);
	
	/* Code (.text) is in another function */
	printf(
		"Task id %d migrated at time %d to processor %x\n", 
		id, 
		MMR_TICK_COUNTER, 
		addr
	);
	
	/* Update task location of tasks of the same app running locally */
	app_update(app, id, addr);
	tcb_remove(tcb);
}

void tm_send_text(tcb_t *tcb, int id, int addr)
{
	packet_t *packet = pkt_slot_get();

	tl_t *mapper = tcb_get_mapper(tcb);
	size_t text_size = tcb_get_text_size(tcb);

	pkt_set_migration_code(
		packet, 
		addr, 
		id, 
		text_size, 
		tl_get_task(mapper), 
		tl_get_addr(mapper)
	);

	/* Align */
	text_size = (text_size + 3) & ~3;

	void *offset = tcb_get_offset(tcb);
	dmni_send(packet, offset, text_size >> 2, false);
}

void tm_send_tcb(tcb_t *tcb, int id, int addr)
{
	/* Send TCB */
	packet_t *packet = pkt_slot_get();

	unsigned received = 0;
	ipipe_t *ipipe = tcb_get_ipipe(tcb);
	if(ipipe != NULL){
		received = ipipe_get_size(ipipe);
		tcb_destroy_ipipe(tcb);
	}

	pkt_set_migration_tcb(
		packet, 
		addr, 
		id, 
		tcb_get_pc(tcb),
		received
	);

	dmni_send(packet, tcb_get_regs(tcb), HAL_MAX_REGISTERS, false);
}

void tm_send_app(tcb_t *tcb, app_t *app, int id, int addr)
{
	size_t task_cnt = app_get_task_cnt(app);

	packet_t *packet = pkt_slot_get();

	pkt_set_migration_app(packet, addr, id, task_cnt);

	dmni_send(packet, app_get_locations(app), task_cnt, false);
}

void tm_send_tl(tcb_t *tcb, list_t *list, unsigned service, int id, int addr)
{
	size_t size = list_get_size(list);

	if(size == 0)
		return;	/* No data available to migrate */

	tl_t *vect = list_vectorize(list, sizeof(tl_t));
	
	if(vect == NULL){
		puts("FATAL: could not allocate memory for TL vector");
		while(1);
	}

	list_clear(list);
	list = NULL;

	packet_t *packet = pkt_slot_get();

	pkt_set_migration_tl(packet, addr, service, id, size);

	dmni_send(packet, vect, (size*sizeof(tl_t)) >> 2, true);
}

void tm_send_opipe(tcb_t *tcb, int id, int addr)
{
	opipe_t *opipe = tcb_get_opipe(tcb);

	if(opipe == NULL)
		return;
	
	// puts("Will send pipe migration\n");
	packet_t *packet = pkt_slot_get();

	size_t size;
	void* buf = opipe_get_buf(opipe, &size);

	pkt_set_migration_pipe(
		packet,
		addr, 
		id, 
		opipe_get_cons_task(opipe),
		size
	);

	size_t align_size = (size + 3) & ~3;

	dmni_send(packet, buf, align_size >> 2, true);

	tcb_destroy_opipe(tcb);
}

void tm_send_stack(tcb_t *tcb, int id, int addr)
{
	/* Get the stack pointer */
	size_t stack_size = MMR_PAGE_SIZE - tcb_get_sp(tcb);

	if(stack_size == 0)
		return;

	/* Align */
	stack_size = (stack_size + 3) & ~3;

	packet_t *packet = pkt_slot_get();

	pkt_set_migration_stack(packet, addr, id, stack_size);

	dmni_send(
		packet, 
		tcb_get_offset(tcb) + MMR_PAGE_SIZE - stack_size, 
		stack_size >> 2, 
		false
	);
}

void tm_send_heap(tcb_t *tcb, int id, int addr)
{
	/* Get the stack pointer */
	void *heap_start = (void*)(
		tcb_get_text_size(tcb) + 
		tcb_get_data_size(tcb) + 
		tcb_get_bss_size(tcb)
	);
	void *heap_end = tcb_get_heap_end(tcb);
	size_t heap_size = heap_end - heap_start;

	if(heap_size == 0)
		return;

	packet_t *packet = pkt_slot_get();

	pkt_set_migration_heap(packet, addr, id, heap_size);

	/* Align to 32 bits */
	heap_size = (heap_size + 3) & ~3;

	dmni_send(
		packet, 
		tcb_get_offset(tcb) + (unsigned)heap_start, 
		heap_size >> 2,
		false
	);
}

void tm_send_data_bss(tcb_t *tcb, int id, int addr)
{
	size_t data_size = tcb_get_data_size(tcb);
	size_t bss_size = tcb_get_bss_size(tcb);
	size_t total_size = data_size + bss_size;

	if(total_size == 0)
		return;

	packet_t *packet = pkt_slot_get();

	pkt_set_migration_data_bss(packet, addr, id, data_size, bss_size);

	total_size = (total_size + 3) & ~3;

	dmni_send(
		packet, 
		tcb_get_offset(tcb) + tcb_get_text_size(tcb), 
		total_size >> 2,
		false
	);
}

void tm_send_sched(tcb_t *tcb, int id, int addr)
{
	packet_t *packet = pkt_slot_get();

	sched_t *sched = tcb_get_sched(tcb);
	pkt_set_migration_sched(
		packet, 
		addr, 
		id, 
		sched_get_period(sched), 
		sched_get_deadline(sched), 
		sched_get_waiting_msg(sched), 
		sched_get_exec_time(sched)
	);

	dmni_send(packet, NULL, 0, false);
}

void tm_abort_task(int id, int addr)
{
	bcast_t packet;

	packet.service = ABORT_TASK;
	packet.src_id = -1;
	packet.payload = id;
	while(!bcast_send(&packet, addr, BR_SVC_TGT));
}

bool _tm_find_app_fnc(void *data, void* cmpval)
{
	tl_t *tl = (tl_t*)data;
	int app_id = *((int*)cmpval);

	return (((tl->task >> 8) & 0xFF) == app_id);
}

void tm_clear_app(int id)
{
	list_entry_t *entry = list_find(&_tms, &id, _tm_find_app_fnc);
	while(entry != NULL){
		tl_t *tl = list_get_data(entry);
		// printf("************* Removed task %d from migration\n", tl->task);
		list_remove(&_tms, entry);
		free(tl);
		entry = list_find(&_tms, &id, _tm_find_app_fnc);
	}
}

bool tm_empty()
{
	return list_empty(&_tms);
}
