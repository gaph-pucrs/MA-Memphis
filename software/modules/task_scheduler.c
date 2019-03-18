/*!\file task_scheduler.c
 * MEMPHIS VERSION - 8.0 - support for RT applications
 *
 * Distribution:  June 2016
 *
 * Created by: Marcelo Ruaro - contact: marcelo.ruaro@acad.pucrs.br
 *
 * Research group: GAPH-PUCRS   -  contact:  fernando.moraes@pucrs.br
 *
 * \brief task_scheduler is a kernel module in charge of implementing
 * task scheduling following a LST (Least Slack Time) algorithm.
 *
 * \detailed The main function of this module is the LST algorithm, which is called by kernel_slave. It
 * returns the pointer for the selected task to execute into the processor.
 * This module is used only by the slave kernel
 */

#include "task_scheduler.h"
#include "../../include/kernel_pkg.h"
#include "../include/plasma.h"
#include "../include/services.h"
#include "packet.h"
#include "utils.h"

Scheduling scheduling[MAX_LOCAL_TASKS];		//!<Scheduling array with its size equal to the max number of task that can execute into the processor

unsigned int time_slice;					//!<Time slice used to configure the processor to generate an interruption
unsigned int schedule_overhead = 500;		//!<Used to dynamically estimate the scheduler overhead
unsigned int instant_overhead;				//!<Used to dynamically estimate the scheduler overhead
unsigned int cpu_utilization = 0;			//!<RT CPU utilization, only filled with RT constraints


/*void send_deadline_miss_report(Scheduling * real_time_task){

	ServiceHeader * p;
	TCB * tcb_ptr;
	tcb_ptr = (TCB *) real_time_task->tcb_ptr;
	puts("### Deadline miss to task "); puts(itoa(tcb_ptr->id)); putsv(" at time ", MemoryRead(TICK_COUNTER));

	p = get_service_header_slot();

	p->header = cluster_master_address;

	p->service = DEADLINE_MISS_REPORT;

	p->task_ID = tcb_ptr->id;

	send_packet(p, 0, 0);
}*/


/**Get the time slice. Useful to provide the kernel slave time slice
 *  \return Global variable time_slice
 */
unsigned int get_time_slice(){
	return time_slice;
}

/**Initializes the scheduling array with valid pointers
 *  \param sched_ptr Pointer to pointer of the scheduler variable into TCB structure
 *  \param tcb_index TCB array index
 */
void init_scheduling_ptr(Scheduling ** sched_ptr, int tcb_index){

	Scheduling * sched_p;

	*sched_ptr = &scheduling[tcb_index];

	sched_p = *sched_ptr;

	sched_p->deadline = NO_DEADLINE;
}

/**Clear a scheduling instance, used when the task is removed from slave processor
 *  \param scheduling_tcb Scheduling pointer to be cleared
 */
void clear_scheduling(Scheduling * scheduling_tcb){

	if (scheduling_tcb->deadline != NO_DEADLINE){

		cpu_utilization -= scheduling_tcb->utilization;

		putsv(" ----> CPU utilization decremented: ", cpu_utilization);
	}

	scheduling_tcb->ready_time = 0;
	scheduling_tcb->status = FREE;
	scheduling_tcb->remaining_exec_time = 0;
	scheduling_tcb->execution_time = 0;
	scheduling_tcb->period = 0;
	scheduling_tcb->deadline = NO_DEADLINE;
	scheduling_tcb->slack_time = 0;
	scheduling_tcb->running_start_time = 0;
	scheduling_tcb->utilization = 0;
	scheduling_tcb->waiting_msg = 0;
}

/**Updates the task slack time. The slack time is the time until the task start the next period,
 * once it finishes it execution time for the current period
 *  \param input_task Scheduling structure of the task to be updated
 *  \param current_time Current system time
 */
void inline update_slack_time(Scheduling * input_task, unsigned int current_time){

	int relative_deadline, time_until_deadline;

	relative_deadline = input_task->ready_time + input_task->deadline;

	time_until_deadline = relative_deadline - current_time;

	if (time_until_deadline < input_task->remaining_exec_time){
		input_task->slack_time = 0;
	} else {
		input_task->slack_time = time_until_deadline - input_task->remaining_exec_time;
	}

#if DEBUG
	putsv(">> Slack time: ", input_task->slack_time);
#endif

}

/**Funcion called by kernel slave when a task call the primitive RealTime.
 * This function set the real-time paramenters for the task.
 *  \param real_time_task Task pointer that change or is setting its RT parameters
 *  \param period Task period in clock cycles
 *  \param deadline Task deadline in clock cycles
 *  \param execution_time Task execution time in clock cycles
 */
void real_time_task(Scheduling * real_time_task, unsigned int period, int deadline, unsigned int execution_time){

	unsigned int current_time, ready_time = 0;

	real_time_task->period = period;
	real_time_task->deadline = deadline;
	real_time_task->execution_time = execution_time;

	current_time = MemoryRead(TICK_COUNTER);

	//If is task already called RealTime
	if (real_time_task->ready_time == 0){

		ready_time = ((unsigned int)(current_time / period)) * period;

		while(ready_time < current_time)
			ready_time+= period;

		real_time_task->ready_time = ready_time;

		real_time_task->status = READY;

		real_time_task->remaining_exec_time = execution_time;

	} else {
		cpu_utilization -= real_time_task->utilization;
	}

	real_time_task->utilization = ((execution_time*100) / period) + 1; //10 is the inherent task overhead

	cpu_utilization += real_time_task->utilization;


#if DEBUG
	putsv("\n---- RealTime called\nAddress: ", (unsigned int) real_time_task);
	putsv("Ready time: ", real_time_task->ready_time);
	putsv("period: ", period);
	putsv("deadline: ", deadline);
	putsv("execution_time: ", execution_time);
	putsv("utilization: ",real_time_task->utilization);
	putsv(" ----> CPU utilization incremented: ", cpu_utilization);
#endif

}

/**A simple round-robin scheduler
 *  \return The next round index
 */
unsigned int round_robin(){

	static unsigned int round_robin = 0;

	if (round_robin == MAX_LOCAL_TASKS-1)
		round_robin = 0;
	else
		round_robin++;

	//putsv("Return index : ", round_robin);
	return round_robin;
}


/**This algorithm try to given an extra time slice to task selected by LST.
 * The main approach is looking for the READY task slack time and the closer end of period
 * \param scheduled Scheduled task pointer
 * \param time Current system time
 */
void inline dynamic_slice_time(Scheduling *scheduled, unsigned int time){

	Scheduling * task;
	unsigned int closer_period, second_LST, end_period;

	closer_period = 0;
	second_LST = 0;

	//---------------------------------------------------
	//Searches for the second task with the lest slack time and the closer end of period
	for(int i=0; i<MAX_LOCAL_TASKS; i++){
		task = &scheduling[i];

		if (task->deadline == NO_DEADLINE || task == scheduled)
			continue;

		if (task->status == SLEEPING || task->waiting_msg){
			end_period = task->ready_time + task->period;
			if (!closer_period || end_period < closer_period){
				closer_period = end_period;
			}
		//Only RUNNING, READY tasks
		} else if (scheduled->slack_time > 0 && (!second_LST || second_LST > task->slack_time)){
			second_LST = task->slack_time;
		}
	}
	//---------------------------------------------------

	//Decides to extend the time slice
	if (second_LST && second_LST < time_slice){
		time_slice = second_LST;
	}

	if (closer_period && (closer_period - time) < time_slice ){
		time_slice = (closer_period - time);
	}

}


/**Updates the dynamic RT parameters for all RT tasks. The dynamic RT parameters are: remaining execution time,
 * status, slack-time
 *  \param current_time Current system time
 */
void inline update_real_time(unsigned int current_time){

	Scheduling * task;

	for(int i=0; i<MAX_LOCAL_TASKS; i++){

		task = &scheduling[i];

		//If the current task a BE task then continue the loop
		if (task->deadline == NO_DEADLINE || task->status == FREE || task->status == MIGRATING || task->status == BLOCKED){
			//If the current task is BE only set it status to READY
			if (task->status == RUNNING && !task->waiting_msg){
				task->status = READY;
			}
			continue;
		}

#if DEBUG
		putsv("\n----\nTask address: ", (unsigned int)task);
		puts("Ready: "); puts(itoa(task->ready_time)); putsv("\t end of period: ", (task->ready_time + task->period));
		//putsv("Status: ", task->status);
#endif

		if (task->status == RUNNING) {
			//remaining_exec_time is equal the time of the task start its execution time until the current time
			task->remaining_exec_time -= (current_time-schedule_overhead) - task->running_start_time;

			//If the task finish its execution, the task must SLEEP untul the end of its period
			if (task->remaining_exec_time <= 0){
				//When the task is set to SPEEP, the function 'update_slack_time' must be called once one last time to update its final slack
				//This slack can be used to monitoring the efective slack time of the task
				task->remaining_exec_time = 0;

				task->status = SLEEPING;

				update_slack_time(task, current_time);

			//However, if task not finish its execution, it goes to READY again
			} else {
				task->status = READY;
			}

			//Check deadline miss
			if (task->waiting_msg == 0 && task->slack_time == 0 && task->remaining_exec_time > (task->execution_time/10))
				puts("#### -------->>>  Deadline miss\n");
			//putsv("Remaining = ", task->remaining_exec_time);
		}

		//Below this region, the task pointer is a valid real-time task
		//The first step is verify it period. If the period has finished, the task must be set to READY
		if (current_time >= (task->ready_time + task->period) ){

#if DEBUG
			putsv("## END of PERIOD\nReady to: ",task->ready_time );
			putsv("remaining exec = ", task->remaining_exec_time);
#endif

			task->ready_time += task->period;

			task->remaining_exec_time = task->execution_time;

			task->status = READY;
		}

		//For all real-time task that are not SLEEPING, the slack time must be updated at each scheduler call
		if (task->status != SLEEPING){
			update_slack_time(task, current_time);
		}
	}
}

/**The LST algorithm called by kernel slave. It select the next RT task with the least slack time,
 * or the next BE task following a round-robin order
 *  \param current_time Current system time
 *  \return The Scheduling pointer of the scheduled task
 */
Scheduling * LST(unsigned int current_time){

	Scheduling * scheduled_task, * task;
	unsigned int end_period;

	instant_overhead = current_time;
//putsv("\nSC: ",current_time);
#if DEBUG
	putsv("\n--------- Scheduler LST ------\n\nCalled at: ", current_time);
	putsv("Overhead: ", (current_time+schedule_overhead));
#endif

	current_time+= schedule_overhead;

	//Updates real-time parameters: slack_time, ready_time, remaining_exe_time, status
	update_real_time(current_time);

	//Initializes scheduled_task pointer as zero
	scheduled_task = 0;

	//Search for a real-time task with the LEAST SLACK TIME
	for(int i=0; i<MAX_LOCAL_TASKS; i++){

		task = &scheduling[i];

		if (task->deadline != NO_DEADLINE && task->status == READY && !task->waiting_msg){

			if (scheduled_task == 0 || (task->slack_time < scheduled_task->slack_time) ){
				scheduled_task = task;
			}
		}
	}

	//If no real-time task are scheduled, selects the next round-robin BE task
	if (scheduled_task == 0){

		for(int i=0; i<MAX_LOCAL_TASKS; i++){

			task = &scheduling[round_robin()];

			if (task->status == READY && !task->waiting_msg) {
				scheduled_task = task;
				break;
			}
		}
	}

	//If at least one task has been selected (BE or RT)
	if (scheduled_task){

		scheduled_task->status = RUNNING;

#if DEBUG
		putsv("Scheduled task: ", (unsigned int) scheduled_task);
#endif

		if (scheduled_task->deadline != NO_DEADLINE){
			time_slice = scheduled_task->remaining_exec_time;
		} else {
			time_slice = MAX_TIME_SLICE;
		}

		//Try given an extra scheduler timer to the real-time task
		dynamic_slice_time(scheduled_task, current_time);

		if (scheduled_task->deadline != NO_DEADLINE){
			//Sets the task running start time to the current time
			scheduled_task->running_start_time = MemoryRead(TICK_COUNTER);
#if DEBUG
			putsv("Running start time: ", scheduled_task->running_start_time);
#endif

		}

	} else { //Schedules Idle

		time_slice = 0;

		//Searches for the next end of period
		for(int i=0; i<MAX_LOCAL_TASKS; i++){
			task = &scheduling[i];

			if (task->deadline == NO_DEADLINE)
				continue;

			if (task->status == SLEEPING || task->waiting_msg){
				end_period = task->ready_time + task->period;
				if (time_slice == 0 || end_period < time_slice){
					time_slice = end_period;
				}
			}
		}
		if (time_slice != 0)
			time_slice = time_slice - current_time;
		else
			time_slice = MAX_TIME_SLICE;

	}

#if DEBUG
		putsv("Time slice: ", time_slice);
#endif

	instant_overhead = MemoryRead(TICK_COUNTER) - instant_overhead;
	schedule_overhead = (unsigned int) (schedule_overhead + instant_overhead) / 2;

	return scheduled_task;
}




