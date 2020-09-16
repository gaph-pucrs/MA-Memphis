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

#include "task_scheduler.h"
#include "task_control.h"
#include "services.h"
#include "packet.h"

tcb_t *current;	//!< TCB pointer used to store the current task executing into processor
unsigned int total_slack_time;	//!< Store the total of the processor idle time
unsigned int last_idle_time; 	//!< Store the last idle time duration

void sched_init()
{
	tcb_t *tcbs = tcb_get();
	for(int i = 0; i < PKG_MAX_LOCAL_TASKS; i++){
		tcbs[i].scheduler.deadline = SCHED_NO_DEADLINE;
		sched_clear(&(tcbs[i].scheduler));
	}

	current = tcb_get_idle();
	total_slack_time = 0;
	last_idle_time = *HAL_TICK_COUNTER;
}

void sched_clear(tcb_t *tcb)
{
	if(tcb->scheduler.deadline != SCHED_NO_DEADLINE){
		cpu_utilization -= tcb->scheduler.utilization;
		putsv(" ----> CPU utilization decremented: ", cpu_utilization);
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
	total_slack_time += *HAL_TICK_COUNTER - last_idle_time;
}

bool sched_is_waiting_request(tcb_t *tcb)
{
	return tcb->scheduler.waiting_msg == SCHED_WAIT_REQUEST;
}

bool sched_is_waiting_request(tcb_t *tcb)
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

	send_packet(packet, 0, 0);

	total_slack_time = 0;
}

void sched_update_idle_time()
{
	last_idle_time = *HAL_TICK_COUNTER;
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

hal_word_t sched_get_period(tcb_t *tcb)
{
	return tcb->scheduler.period;
}

int sched_get_deadline(tcb_t *tcb)
{
	return tcb->scheduler.deadline;
}

hal_word_t sched_get_exec_time(tcb_t *tcb)
{
	return tcb->scheduler.execution_time;
}

void sched_set_remaining_time(tcb_t *tcb, hal_word_t timeslice)
{
	tcb->scheduler.remaining_exec_time = timeslice;
}












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




