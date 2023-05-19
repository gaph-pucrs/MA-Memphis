/**
 * MA-Memphis
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
 * @details 
 * The main function of this module is the LST algorithm, which is 
 * called by kernel. It returns the pointer for the selected task to execute 
 * by the processor.
 */

#include "task_scheduler.h"

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

#include <mutils/list.h>
#include <memphis/services.h>
#include <memphis/monitor.h>

#include "interrupts.h"
#include "task_control.h"
#include "dmni.h"
#include "llm.h"
#include "mmr.h"

static const unsigned SCHED_MAX_TIME_SLICE = 16318;	//!< Standard time slice value for task execution
static const unsigned REPORT_SCHEDULER = 0x40000;
static const unsigned REPORT_IDLE = 0x80000;
static const unsigned REPORT_INTERRUPTION = 0x10000;
static const int SCHED_NO_DEADLINE = -1;	//!< A task that is best-effort have its deadline variable equal to -1

tcb_t *current = NULL;

unsigned total_slack_time = 0;	//!< Store the total of the processor idle time
unsigned cpu_utilization = 0;	//!< RT CPU utilization, only filled with RT constraints
unsigned time_slice = 0;		//!< Time slice used to configure the processor to generate an interruption
unsigned last_idle_time = 0;	//!< Store the last idle time duration

list_t _scheds;

void sched_init()
{
	list_init(&_scheds);

	last_idle_time = MMR_TICK_COUNTER;
}

sched_t *sched_emplace_back(tcb_t *tcb)
{
	sched_t *sched = malloc(sizeof(sched_t));

	if(sched == NULL)
		return NULL;

	if(list_push_back(&_scheds, sched) == NULL){
		free(sched);
		return NULL;
	}

	sched->status = SCHED_READY;
	sched->waiting_msg = SCHED_WAIT_NO;
	sched->last_monitored = 0;

	sched->exec_time = 0;
	sched->period = 0;
	sched->deadline = SCHED_NO_DEADLINE;

	sched->ready_time = 0;
	sched->remaining_exec_time = SCHED_MAX_TIME_SLICE;
	sched->slack_time = 0;
	sched->running_start_time = 0;
	sched->utilization = 0;

	sched->tcb = tcb;

	tcb_set_sched(tcb, sched);

	return sched;
}

bool sched_is_idle()
{
	return (current == NULL);
}

void sched_remove(sched_t *sched)
{
	if(sched->deadline != SCHED_NO_DEADLINE){
		cpu_utilization -= sched->utilization;
		printf(" ----> CPU utilization decremented by %d, now is %d\n", sched->utilization, cpu_utilization);
	}

	list_entry_t *entry = list_find(&_scheds, sched, NULL);
	if(entry != NULL)
		list_remove(&_scheds, entry);

	free(sched);
}

tcb_t *sched_get_current_tcb()
{
	return current;
}

void sched_update_slack_time()
{
	total_slack_time += MMR_TICK_COUNTER - last_idle_time;
}

bool sched_is_waiting_msgreq(sched_t *sched)
{
	return (sched->waiting_msg == SCHED_WAIT_REQUEST);
}

bool sched_is_waiting_dav(sched_t *sched)
{
	return (sched->waiting_msg == SCHED_WAIT_DATA_AV);
}

bool sched_is_waiting_delivery(sched_t *sched)
{
	return (sched->waiting_msg == SCHED_WAIT_DELIVERY);
}

void sched_release_wait(sched_t *sched)
{
	sched->waiting_msg = SCHED_WAIT_NO;
}

void sched_update_idle_time()
{
	last_idle_time = MMR_TICK_COUNTER;
	MMR_SCHEDULING_REPORT = REPORT_IDLE;
}

void sched_set_wait_msgreq(sched_t *sched)
{
	sched->waiting_msg = SCHED_WAIT_REQUEST;
}

void sched_set_wait_dav(sched_t *sched)
{
	sched->waiting_msg = SCHED_WAIT_DATA_AV;
}

void sched_set_wait_msgdlvr(sched_t *sched)
{
	sched->waiting_msg = SCHED_WAIT_DELIVERY;
}

unsigned sched_get_period(sched_t *sched)
{
	return sched->period;
}

int sched_get_deadline(sched_t *sched)
{
	return sched->deadline;
}

unsigned sched_get_exec_time(sched_t *sched)
{
	return sched->exec_time;
}

void _sched_dynamic_slice_time(sched_t *scheduled, unsigned time)
{
	unsigned closer_period = 0;
	unsigned second_lst = 0;

	/* Searches for the second task with the least slack time and the closer end of period */
	list_entry_t *entry = list_front(&_scheds);

	while(entry != NULL){
		sched_t *sched = list_get_data(entry);

		if(sched->deadline == SCHED_NO_DEADLINE || sched == scheduled){
			entry = list_next(entry);
			continue;
		}

		if(sched->status == SCHED_SLEEPING || sched->waiting_msg != SCHED_WAIT_NO){
			unsigned end_period = sched->ready_time + sched->period;
			if(closer_period == 0 || end_period < closer_period)
				closer_period = end_period;
			
		} else if(
			scheduled->slack_time && 
			(second_lst == 0 || second_lst > sched->slack_time)
		){
			/* Only RUNNING, READY tasks */
			second_lst = sched->slack_time;
		}

		entry = list_next(entry);
	}

	/* Decides to extend the time slice */
	if(second_lst > 0 && second_lst < time_slice)
		time_slice = second_lst;

	if(closer_period > 0 && (closer_period - time) < time_slice)
		time_slice = closer_period - time;

}

void _sched_idle_slice_time(unsigned time)
{
	time_slice = 0;

	/* Searches for the next end of period */
	list_entry_t *entry = list_front(&_scheds);
	while(entry != NULL){
		sched_t *sched = list_get_data(entry);

		if(sched->deadline == SCHED_NO_DEADLINE){
			entry = list_next(entry);
			continue;
		}

		if(sched->status == SCHED_SLEEPING || sched->waiting_msg != SCHED_WAIT_NO){
			unsigned end_period = sched->ready_time + sched->period;
			if(time_slice == 0 || end_period < time_slice)
				time_slice = end_period;
		}

		entry = list_next(entry);
	}

	if(time_slice)
		time_slice = time_slice - time;
	else
		time_slice = SCHED_MAX_TIME_SLICE;
}

void _sched_update_task_slack_time(sched_t *sched, unsigned current_time)
{
	int relative_deadline = sched->ready_time + sched->deadline;
	int time_until_deadline = relative_deadline - current_time;

	if(time_until_deadline < sched->remaining_exec_time)
		sched->slack_time = 0;
	else
		sched->slack_time = time_until_deadline - sched->remaining_exec_time;
}

void _sched_rt_update(unsigned current_time, unsigned schedule_overhead)
{
	bool should_monitor = llm_has_monitor(MON_QOS);

	list_entry_t *entry = list_front(&_scheds);
	while(entry != NULL){
		sched_t *sched = list_get_data(entry);

		/* If the current task is a BEST EFFORT task then continue the loop */
		if(sched->deadline == SCHED_NO_DEADLINE){
			/* If the current task is BEST EFFORT only set it status to READY */
			if(sched->status == SCHED_RUNNING && sched->waiting_msg == SCHED_WAIT_NO)
				sched->status = SCHED_READY;

			entry = list_next(entry);
			continue;
		}

		if(sched->status == SCHED_RUNNING){
			/* Remaining execution time is equal to the time that the task 
								started its execution until the current time */
			sched->remaining_exec_time -= 
				(current_time - schedule_overhead) - sched->running_start_time;

			/* If the task has finished its execution, it must SLEEP until the end of its period */
			if(sched->remaining_exec_time <= 0){
				/* When the task is set to SLEEP, the function 
					'sched_update_slack_time' must be called once one last time 
													to update its final slack */
				/* This slack can be used to monitor the efective slack time of the task */
				sched->remaining_exec_time = 0;
				sched->status = SCHED_SLEEPING;

				_sched_update_task_slack_time(sched, current_time);

			} else {
				/* However, if the task has not finished its execution, it goes to READY again */
				sched->status = SCHED_READY;
			}

			/* Monitor RT task after update */
			if(should_monitor){
				int id = tcb_get_id(sched->tcb);
				int appid = id >> 8;
				int mig_pe = tcb_get_migrate_addr(sched->tcb);
				
				if(appid != 0 && mig_pe == -1 && sched->waiting_msg == SCHED_WAIT_NO){
					llm_rt(
						&(sched->last_monitored), 
						id, 
						sched->slack_time, 
						sched->remaining_exec_time
					);
				}
			}
		}

		/* Below this region, the task chosen is a valid real-time task */
		/* The first step is verify its period. If the period has finished, the task must be set to READY */
		if(current_time >= (sched->ready_time + sched->period)){
			sched->ready_time += sched->period;
			sched->remaining_exec_time = sched->exec_time;

			/* End of period -- ready to run */
			sched->status = SCHED_READY;
		}

		/* For all real-time task that are not SLEEPING, the slack time must be updated at each scheduler call */
		if(sched->status != SCHED_SLEEPING)
			_sched_update_task_slack_time(sched, current_time);

		entry = list_next(entry);
	}
}

sched_t *_sched_lst(unsigned current_time)
{
	static sched_t *_last_scheduled = NULL;
	static unsigned schedule_overhead = 500;	//!<Used to dynamically estimate the scheduler overhead

	unsigned instant_overhead = current_time;
	current_time += schedule_overhead;

	/* Updates real-time parameters: slack_time, ready_time, remaining_exe_time, status */
	_sched_rt_update(current_time, schedule_overhead);

	sched_t *scheduled = NULL;

	list_entry_t *entry = list_front(&_scheds);
	while(entry != NULL){
		sched_t *sched = list_get_data(entry);
		
		if(
			sched->deadline != SCHED_NO_DEADLINE && 
			sched->status == SCHED_READY && 
			sched->waiting_msg == SCHED_WAIT_NO
		){
			if(scheduled == NULL || (sched->slack_time < scheduled->slack_time))
				scheduled = sched;
		}

		entry = list_next(entry);
	}

	/* If no real-time tasks are scheduled, selects the next round-robin BEST EFFORT task */
	if(scheduled == NULL){
		/* Fire the round robin */
		entry = list_find(&_scheds, _last_scheduled, NULL);
		
		if(entry != NULL)
			entry = list_next(entry);

		while(entry != NULL){
			sched_t *sched = list_get_data(entry);

			if(sched->status == SCHED_READY && sched->waiting_msg == SCHED_WAIT_NO){
				scheduled = sched;
				break;
			}

			entry = list_next(entry);
		}

		if(scheduled == NULL){
			entry = list_front(&_scheds);
			while(entry != NULL){
				sched_t *sched = list_get_data(entry);

				if(sched->status == SCHED_READY && sched->waiting_msg == SCHED_WAIT_NO){
					scheduled = sched;
					break;
				}

				if(sched == _last_scheduled)
					break;
				
				entry = list_next(entry);
			}
		}
	}

	/* If at least one task has been selected (BEST EFFORT or REAL TIME) */
	if(scheduled != NULL){
		_last_scheduled = scheduled;

		scheduled->status = SCHED_RUNNING;

		if(scheduled->deadline != SCHED_NO_DEADLINE)
			time_slice = scheduled->remaining_exec_time;
		else
			time_slice = SCHED_MAX_TIME_SLICE;

		/* Try given an extra scheduler timer to the real-time task */
		_sched_dynamic_slice_time(scheduled, current_time);

		if(scheduled->deadline != SCHED_NO_DEADLINE){
			/* Sets the task running start time to the current time */
			scheduled->running_start_time = MMR_TICK_COUNTER;
		}

	} else { 
		/* Schedules Idle */
		_sched_idle_slice_time(current_time);
	}

	instant_overhead = MMR_TICK_COUNTER - instant_overhead;
	schedule_overhead = (schedule_overhead + instant_overhead) >> 1;

	return scheduled;
}

void sched_run()
{
	// puts("Scheduler called!");
	unsigned scheduler_call_time = MMR_TICK_COUNTER;

	MMR_SCHEDULING_REPORT = REPORT_SCHEDULER;

	sched_t *sched = _sched_lst(scheduler_call_time);
	
	if(sched != NULL){
		current = sched->tcb;
		sched_report(tcb_get_id(current));
	} else {
		current = NULL;
		sched_update_idle_time();
	}

	MMR_TIME_SLICE = time_slice;
}

void sched_real_time_task(sched_t *sched, unsigned period, int deadline, unsigned execution_time)
{
	unsigned current_time = MMR_TICK_COUNTER;

	sched->period = period;
	sched->deadline = deadline;
	sched->exec_time = execution_time;

	/* If task has already called RealTime */
	if(sched->ready_time == 0){
		unsigned ready_time = ((unsigned)(current_time / period)) * period;

		while(ready_time < current_time)
			ready_time += period;

		sched->ready_time = ready_time;
		sched->remaining_exec_time = execution_time;

		sched->status = SCHED_READY;

	} else {
		cpu_utilization -= sched->utilization;
	}

	sched->utilization = ((execution_time*100) / period) + 1; //10 is the inherent task overhead

	cpu_utilization += sched->utilization;
}

sched_wait_t sched_get_waiting_msg(sched_t *sched)
{
	return sched->waiting_msg;
}

void sched_set_waiting_msg(sched_t *sched, sched_wait_t waiting_msg)
{
	sched->waiting_msg = waiting_msg;
}

void sched_report_interruption()
{
	MMR_SCHEDULING_REPORT = REPORT_INTERRUPTION;
}

void sched_report(int id)
{
	MMR_SCHEDULING_REPORT = id;
}
