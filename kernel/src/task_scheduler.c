/**
 * 
 * @file task_scheduler.c
 *
 * @author Marcelo Ruaro (marcelo.ruaro@acad.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date June 2016
 * 
 * @brief Defines the scheduling structures.
 * 
 * @details The main function of this module is the LST algorithm, which is 
 * called by kernel. It returns the pointer for the selected task to execute 
 * by the processor.
 */

#include <stddef.h>

#include "task_scheduler.h"
#include "packet.h"
#include "services.h"
#include "task_migration.h"
#include "llm.h"
#include "stdio.h"

const int SCHED_NO_DEADLINE = -1;			//!< A task that is best-effor have its deadline variable equal to -1

tcb_t *current;						//!< TCB pointer used to store the current task executing into processor
unsigned int total_slack_time;		//!< Store the total of the processor idle time
unsigned int last_idle_time; 		//!< Store the last idle time duration
unsigned int cpu_utilization = 0;	//!< RT CPU utilization, only filled with RT constraints
unsigned int time_slice;			//!< Time slice used to configure the processor to generate an interruption

void sched_init()
{
	tcb_t *tcbs = tcb_get();
	for(int i = 0; i < PKG_MAX_LOCAL_TASKS; i++){
		tcbs[i].scheduler.deadline = SCHED_NO_DEADLINE;
		sched_clear(&tcbs[i]);
	}

	current = tcb_get_idle();
	total_slack_time = 0;
	last_idle_time = MMR_TICK_COUNTER;
}

void sched_clear(tcb_t *tcb)
{
	if(tcb->scheduler.deadline != SCHED_NO_DEADLINE){
		cpu_utilization -= tcb->scheduler.utilization;
		printf(" ----> CPU utilization decremented by %d, now is %d\n", tcb->scheduler.utilization, cpu_utilization);
	}

	tcb->scheduler.ready_time = 0;
	tcb->scheduler.status = SCHED_FREE;
	tcb->scheduler.remaining_exec_time = 0;
	tcb->scheduler.execution_time = 0;
	tcb->scheduler.period = 0;
	tcb->scheduler.deadline = SCHED_NO_DEADLINE;
	tcb->scheduler.slack_time = 0;
	tcb->scheduler.running_start_time = 0;
	tcb->scheduler.utilization = 0;
	tcb->scheduler.waiting_msg = 0;
}

tcb_t *sched_get_current()
{
	return current;
}

bool sched_is_idle()
{
	return (current == tcb_get_idle());
}

void sched_update_slack_time()
{
	total_slack_time += MMR_TICK_COUNTER - last_idle_time;
}

bool sched_is_waiting_request(tcb_t *tcb)
{
	return tcb->scheduler.waiting_msg == SCHED_WAIT_REQUEST;
}

bool sched_is_waiting_data_av(tcb_t *tcb)
{
	return tcb->scheduler.waiting_msg == SCHED_WAIT_DATA_AV;
}

bool sched_is_waiting_delivery(tcb_t *tcb)
{
	return tcb->scheduler.waiting_msg == SCHED_WAIT_DELIVERY;
}

bool sched_is_blocked(tcb_t *tcb)
{
	return tcb->scheduler.status == SCHED_BLOCKED;
}

void sched_release_wait(tcb_t *tcb)
{
	tcb->scheduler.waiting_msg = 0;
}

void sched_release(tcb_t *tcb)
{
	tcb->scheduler.status = SCHED_READY;
}

void sched_report_slack_time()
{
	packet_t *packet = pkt_slot_get();

	// packet->header = cluster_master_address;
	packet->service = SLACK_TIME_REPORT;
	packet->cpu_slack_time = ((total_slack_time*100) / PKG_SLACK_TIME_WINDOW);

	pkt_send(packet, NULL, 0);

	total_slack_time = 0;
}

void sched_update_idle_time()
{
	last_idle_time = MMR_TICK_COUNTER;
}

int sched_get_current_id()
{
	if(sched_is_idle())
		return -1;
	else
		return current->id;
}

void sched_block(tcb_t *tcb)
{
	tcb->scheduler.status = SCHED_BLOCKED;
}

void sched_set_wait_request(tcb_t *tcb)
{
	tcb->scheduler.waiting_msg = SCHED_WAIT_REQUEST;
}

void sched_set_wait_data_av(tcb_t *tcb)
{
	tcb->scheduler.waiting_msg = SCHED_WAIT_DATA_AV;
}

void sched_set_wait_delivery(tcb_t *tcb)
{
	tcb->scheduler.waiting_msg = SCHED_WAIT_DELIVERY;
}

unsigned int sched_get_period(tcb_t *tcb)
{
	return tcb->scheduler.period;
}

int sched_get_deadline(tcb_t *tcb)
{
	return tcb->scheduler.deadline;
}

unsigned int sched_get_exec_time(tcb_t *tcb)
{
	return tcb->scheduler.execution_time;
}

void sched_set_remaining_time(tcb_t *tcb, unsigned int timeslice)
{
	tcb->scheduler.remaining_exec_time = timeslice;
}

void sched_run()
{
	// puts("Scheduler called!\n");
	unsigned int scheduler_call_time = MMR_TICK_COUNTER;

	MMR_SCHEDULING_REPORT = MMR_SCHEDULER;

	if(tcb_need_migration(current) && current->scheduler.status == SCHED_RUNNING && current->scheduler.waiting_msg == 0)
		tm_migrate(current);

	tcb_t *scheduled = sched_lst(scheduler_call_time);
	// printf("Scheduled TCB addr is %x\n", (unsigned)scheduled);

	if(scheduled){
		current = scheduled;
		MMR_SCHEDULING_REPORT = tcb_get_id(current);
		// printf("Current TCB addr is %x\n", (unsigned)current);
	} else {
		/* Schedules the idle task */
		current = tcb_get_idle();
		last_idle_time = MMR_TICK_COUNTER;
        MMR_SCHEDULING_REPORT = MMR_IDLE;
	}

	MMR_TIME_SLICE = time_slice;

	/* Set the scheduler interrupt mask */
	MMR_IRQ_MASK |= MMR_IRQ_SCHEDULER;
}

tcb_t *sched_lst(unsigned int current_time)
{
	static unsigned int schedule_overhead = 500;	//!<Used to dynamically estimate the scheduler overhead

	unsigned int instant_overhead = current_time;
	current_time += schedule_overhead;

	/* Updates real-time parameters: slack_time, ready_time, remaining_exe_time, status */
	sched_rt_update(current_time, schedule_overhead);

	tcb_t *scheduled = NULL;
	tcb_t *tasks = tcb_get();

	/* Search for a real-time task with the LEAST SLACK TIME */
	for(int i = 0; i < PKG_MAX_LOCAL_TASKS; i++){
		if(tasks[i].scheduler.deadline != SCHED_NO_DEADLINE && tasks[i].scheduler.status == SCHED_READY && !tasks[i].scheduler.waiting_msg){
			if(!scheduled || (tasks[i].scheduler.slack_time < scheduled->scheduler.slack_time))
				scheduled = &tasks[i];
		}
	}

	/* If no real-time tasks are scheduled, selects the next round-robin BEST EFFORT task */
	if(!scheduled){
		for(int i = 0; i < PKG_MAX_LOCAL_TASKS; i++){
			/* Fire the round robin */
			unsigned int rr = sched_round_robin();

			if (tasks[rr].scheduler.status == SCHED_READY && !tasks[rr].scheduler.waiting_msg) {
				scheduled = &tasks[rr];
				break;
			}
		}
	}

	/* If at least one task has been selected (BEST EFFORT or REAL TIME) */
	if(scheduled){
		scheduled->scheduler.status = SCHED_RUNNING;

		if(scheduled->scheduler.deadline != SCHED_NO_DEADLINE)
			time_slice = scheduled->scheduler.remaining_exec_time;
		else
			time_slice = SCHED_MAX_TIME_SLICE;

		/* Try given an extra scheduler timer to the real-time task */
		sched_dynamic_slice_time(scheduled, current_time);

		if(scheduled->scheduler.deadline != SCHED_NO_DEADLINE){
			/* Sets the task running start time to the current time */
			scheduled->scheduler.running_start_time = MMR_TICK_COUNTER;
		}

	} else { 
		/* Schedules Idle */
		time_slice = 0;

		/* Searches for the next end of period */
		for(int i = 0; i < PKG_MAX_LOCAL_TASKS; i++){
			if(tasks[i].scheduler.deadline == SCHED_NO_DEADLINE)
				continue;

			if(tasks[i].scheduler.status == SCHED_SLEEPING || tasks[i].scheduler.waiting_msg){
				unsigned int end_period = tasks[i].scheduler.ready_time + tasks[i].scheduler.period;
				if (time_slice == 0 || end_period < time_slice)
					time_slice = end_period;
			}
		}
		if(time_slice)
			time_slice = time_slice - current_time;
		else
			time_slice = SCHED_MAX_TIME_SLICE;
	}

	instant_overhead = MMR_TICK_COUNTER - instant_overhead;
	schedule_overhead = (schedule_overhead + instant_overhead) / 2;

	return scheduled;
}

unsigned int sched_round_robin(){
	static unsigned int round_robin = 0;

	round_robin++;
	round_robin %= PKG_MAX_LOCAL_TASKS;

	return round_robin;
}

void sched_rt_update(unsigned int current_time, unsigned int schedule_overhead)
{
	tcb_t *tasks = tcb_get();

	for(int i = 0; i < PKG_MAX_LOCAL_TASKS; i++){
		/* If the current task is a BEST EFFORT task then continue the loop */
		if(tasks[i].scheduler.deadline == SCHED_NO_DEADLINE || tasks[i].scheduler.status == SCHED_FREE || tasks[i].scheduler.status == SCHED_MIGRATING || tasks[i].scheduler.status == SCHED_BLOCKED){
			/* If the current task is BEST EFFORT only set it status to READY */
			if(tasks[i].scheduler.status == SCHED_RUNNING && !tasks[i].scheduler.waiting_msg)
				tasks[i].scheduler.status = SCHED_READY;

			continue;
		}

		if(tasks[i].scheduler.status == SCHED_RUNNING){
			/* Remaining execution time is equal to the time that the task started its execution until the current time */
			tasks[i].scheduler.remaining_exec_time -= (current_time - schedule_overhead) - tasks[i].scheduler.running_start_time;

			/* If the task has finished its execution, it must SLEEP until the end of its period */
			if(tasks[i].scheduler.remaining_exec_time <= 0){
				/* When the task is set to SLEEP, the function 'sched_update_slack_time' must be called once one last time to update its final slack */
				/* This slack can be used to monitor the efective slack time of the task */
				tasks[i].scheduler.remaining_exec_time = 0;
				tasks[i].scheduler.status = SCHED_SLEEPING;

				sched_update_task_slack_time(&tasks[i], current_time);

			} else {
				/* However, if the task has not finished its execution, it goes to READY again */
				tasks[i].scheduler.status = SCHED_READY;
			}

			/* Check deadline miss */
			// if(!tasks[i].scheduler.waiting_msg && !tasks[i].scheduler.slack_time && tasks[i].scheduler.remaining_exec_time > (tasks[i].scheduler.execution_time/10)){
			// 	putsv("#### -------->>>  Deadline miss at task ", tasks[i].id);
			// 	// putsvsv("P = ", tasks[i].scheduler.period, "; D = ", tasks[i].scheduler.deadline);
			// 	// putsvsv("; ET = ", tasks[i].scheduler.execution_time, "; RT = ", tasks[i].scheduler.remaining_exec_time);
			// 	// putsv("; S = ", tasks[i].scheduler.slack_time);
			// 	// puts("\n");
			// }
		}

		/* Below this region, the task chosen is a valid real-time task */
		/* The first step is verify its period. If the period has finished, the task must be set to READY */
		if(current_time >= (tasks[i].scheduler.ready_time + tasks[i].scheduler.period)){
			tasks[i].scheduler.ready_time += tasks[i].scheduler.period;
			tasks[i].scheduler.remaining_exec_time = tasks[i].scheduler.execution_time;

			/* Fim do período -- pronto para executar */
			tasks[i].scheduler.status = SCHED_READY;
		}

		/* For all real-time task that are not SLEEPING, the slack time must be updated at each scheduler call */
		if(tasks[i].scheduler.status != SCHED_SLEEPING)
			sched_update_task_slack_time(&tasks[i], current_time);
		
	}

	/* Monitor all RT tasks */
	llm_rt(tasks);
}

void sched_update_task_slack_time(tcb_t *task, unsigned int current_time)
{
	int relative_deadline = task->scheduler.ready_time + task->scheduler.deadline;
	int time_until_deadline = relative_deadline - current_time;

	if(time_until_deadline < task->scheduler.remaining_exec_time)
		task->scheduler.slack_time = 0;
	else
		task->scheduler.slack_time = time_until_deadline - task->scheduler.remaining_exec_time;
}

void sched_dynamic_slice_time(tcb_t *scheduled, unsigned int time)
{
	tcb_t *tasks = tcb_get();
	unsigned int closer_period = 0;
	unsigned int second_lst = 0;
	/* Searches for the second task with the least slack time and the closer end of period */
	for(int i = 0; i < PKG_MAX_LOCAL_TASKS; i++){
		if (tasks[i].scheduler.deadline == SCHED_NO_DEADLINE || &tasks[i] == scheduled)
			continue;

		if(tasks[i].scheduler.status == SCHED_SLEEPING || tasks[i].scheduler.waiting_msg){
			unsigned int end_period = tasks[i].scheduler.ready_time + tasks[i].scheduler.period;
			if(!closer_period || end_period < closer_period)
				closer_period = end_period;
			
		/* Only RUNNING, READY tasks */
		} else if(scheduled->scheduler.slack_time && (!second_lst || second_lst > tasks[i].scheduler.slack_time)){
			second_lst = tasks[i].scheduler.slack_time;
		}
	}

	/* Decides to extend the time slice */
	if(second_lst && second_lst < time_slice)
		time_slice = second_lst;

	if(closer_period && (closer_period - time) < time_slice)
		time_slice = (closer_period - time);

}

void sched_real_time_task(tcb_t *task, unsigned int period, int deadline, unsigned int execution_time)
{
	unsigned int current_time = MMR_TICK_COUNTER;

	task->scheduler.period = period;
	task->scheduler.deadline = deadline;
	task->scheduler.execution_time = execution_time;

	/* If task has already called RealTime */
	if(!task->scheduler.ready_time){
		unsigned int ready_time = ((unsigned int)(current_time / period)) * period;

		while(ready_time < current_time)
			ready_time += period;

		task->scheduler.ready_time = ready_time;
		task->scheduler.status = SCHED_READY;
		task->scheduler.remaining_exec_time = execution_time;

	} else {
		cpu_utilization -= task->scheduler.utilization;
	}

	task->scheduler.utilization = ((execution_time*100) / period) + 1; //10 is the inherent task overhead

	cpu_utilization += task->scheduler.utilization;
}
