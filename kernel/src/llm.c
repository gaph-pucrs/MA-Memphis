/**
 * MA-Memphis
 * @file llm.c
 *
 * @author Angelo Elias Dalzotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date March 2020
 * 
 * @brief Implements the Low-Level Monitor for Management Application support.
 */

#include "llm.h"

#include "syscall.h"
#include "broadcast.h"
#include "monitor.h"
#include "interrupts.h"

#include "services.h"

#include "stdlib.h"

observer_t observers[MON_MAX];

void llm_init()
{
	for(int i = 0; i < MON_MAX; i++)
		observers[i].addr = -1;
}

void llm_set_observer(enum MONITOR_TYPE type, int addr)
{
	int pe_addr = MMR_NI_CONFIG;
	uint8_t pe_x = pe_addr >> 8;
	uint8_t pe_y = pe_addr & 0xFF;
	uint8_t obs_x = addr >> 8;
	uint8_t obs_y = addr & 0xFF;
	uint16_t dist = abs(pe_x - obs_x) + abs(pe_y - obs_y);

	if(observers[type].addr == -1 || observers[type].dist > dist){
		observers[type].addr = addr;
		observers[type].dist = dist;
	}
}

void llm_clear_table(tcb_t *task)
{
	int id = task->id & 0xFFFF;
	os_clear_mon_table(id);

	br_packet_t packet;
	packet.service = CLEAR_MON_TABLE;
	packet.src_id = -1;

	packet.payload = id;

	while(!br_send(&packet, MMR_NI_CONFIG, BR_SVC_ALL));
}

void llm_rt(tcb_t *tasks)
{
	static unsigned last_rt[PKG_MAX_LOCAL_TASKS];
	if(observers[MON_QOS].addr == -1)
		return;

	unsigned now = MMR_TICK_COUNTER;

	for(int i = 0; i < PKG_MAX_LOCAL_TASKS; i++){
		int id = tasks[i].id;
		if(id == -1 || (id >> 8) == 0 || tasks[i].scheduler.deadline == -1 || tasks[i].proc_to_migrate != -1)
			continue; /* Don't send MA task status or non-RT tasks or non-existent tasks or tasks marked to migrate */

		if(!tasks[i].scheduler.waiting_msg && now - last_rt[i] >= PKG_MONITOR_INTERVAL_QOS/PKG_MONITOR_RATE_QOS){ /* Update X times faster than the real time observer */
				br_packet_t packet;
				packet.service = MONITOR;
				packet.src_id = id;
				packet.payload = tasks[i].scheduler.slack_time - tasks[i].scheduler.remaining_exec_time;
				if(br_send(&packet, observers[MON_QOS].addr, MON_QOS))
					last_rt[i] = now;
		}
	}
}
