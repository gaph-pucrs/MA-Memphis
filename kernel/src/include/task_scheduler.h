/**
 * MA-Memphis
 * @file task_scheduler.h
 *
 * @author Marcelo Ruaro (marcelo.ruaro@acad.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date June 2016
 * 
 * @brief Declares the scheduling structures.
 * 
 * @details 
 * A given task can be BE (Best-Effort) or RT (Real-Time).
 * BE task has no timing constraints.
 * RT has the timing constraints defined according the diagram below
 * Periodic task real-time parameters
 * |<------------ period ------------------>|
 * |										|
 * |<------------deadline------------>|	    |
 * |							      | 	|
 * |            		  		      |		|
 * | _______________________          |		|
 * || 	execution time		|<-slack->|		|
 * ||_______________________|_________|_____|
 * ^			time ->
 * |
 * ready time
 */

#pragma once

#include <stdbool.h>

/* Forward Declaration */
typedef struct _tcb tcb_t;

#define SCHED_MAX_TIME_SLICE 16318	//!< Standard time slice value for task execution
#define REPORT_SCHEDULER	0x40000
#define REPORT_IDLE			0x80000


/**
 * @brief Reasons the task can be blocked for a message synchronization.
 */
enum waiting_status {
	SCHED_WAIT_DELIVERY = 1,
	SCHED_WAIT_DATA_AV,
	SCHED_WAIT_REQUEST
};

/**
 * @brief Task scheduling status.
 */
enum scheduler_status {
	SCHED_FREE = -1,
	SCHED_READY = 0,		//!< Task is in ready to execute
	SCHED_RUNNING = 2,	//!< Task is running in the CPU
	SCHED_MIGRATING,		//!< Task is being migrated
	SCHED_BLOCKED,		//!< Task is blocked waiting for the manager send a TASK_RELEASE packet
	SCHED_SLEEPING		//!< Task already executed all its execution time and is sleeping until the end of period
};

/**
 * @brief This structure stores variables useful to manage the task scheduling 
 * (for BE or RT).
 * 
 * @details Some of the values are only used for RT tasks
 */
typedef struct _scheduler {
	char			status;				//!< Task scheduling status
	unsigned int	execution_time;		//!< Task execution time in clock cycles
	unsigned int 	period;				//!< Task period in clock cycles
	int 			deadline;			//!< Task deadline in clock cycles, for BE task is set to -1
	unsigned int 	ready_time;			//!< Time in clock cycles that task becomes ready
	int 			remaining_exec_time;//!< Task remaining execution time in clock cycles
	unsigned int 	slack_time;			//!< Task slack time in clock cycles
	unsigned int 	running_start_time;	//!< Task running start time in clock cycles
	unsigned int 	utilization;		//!< Task CPU utilization in percentage
	unsigned int 	waiting_msg;		//!< Signals when task is waiting a message from a producer task
} scheduler_t;

/**
 * @brief Initializes the scheduler for the first time.
 */
void sched_init();

/**
 * @brief Clear a scheduler structure.
 * 
 * @details Must be called when a task is removed from processor.
 * 
 * @param tcb Pointer to the TCB
 */
void sched_clear(tcb_t *tcb);

/**
 * @brief Get the current scheduled task.
 * 
 * @return Pointer to the TCB of the scheduled task.
 */
tcb_t *sched_get_current();

/**
 * @brief Checks if the scheduler is idle.
 * 
 * @return True if scheduled task is the idle one.
 */
bool sched_is_idle();

/**
 * @brief Updates the total slack time.
 */
void sched_update_slack_time();

/**
 * @brief Checks if the task is waiting for a MESSAGE_REQUEST
 * 
 * @param tcb Pointer to the task TCB
 * 
 * @return True if the task is waiting
 */
bool sched_is_waiting_request(tcb_t *tcb);

/**
 * @brief Checks if the task is waiting for a DATA_AV
 * 
 * @param tcb Pointer to the task TCB
 * 
 * @return True if the task is waiting
 */
bool sched_is_waiting_data_av(tcb_t *tcb);

/**
 * @brief Checks if the task is waiting for a message delivery
 * 
 * @param tcb Pointer to the task TCB
 * 
 * @return True if the task is waiting
 */
bool sched_is_waiting_delivery(tcb_t *tcb);

/**
 * @brief Checks if the task is blocked
 * 
 * @param tcb Pointer to the TCB
 * 
 * @return True if the task is blocked
 */
bool sched_is_blocked(tcb_t *tcb);

/**
 * @brief Releases the task from a waiting message condition
 * 
 * @param tcb Pointer to the task TCB
 */
void sched_release_wait(tcb_t *tcb);

/**
 * @brief Releases a task for execution
 * 
 * @param tcb Pointer to the TCB
 */
void sched_release(tcb_t *tcb);

/**
 * @brief Report the slack time
 */
void sched_report_slack_time();

/**
 * @brief Updates the scheduler idle counter
 */
void sched_update_idle_time();

/**
 * @brief Gets the ID of the running task
 * 
 * @return ID of the task. -1 if idle task is running.
 */
int sched_get_current_id();

/**
 * @brief Blocks a task from executing until it receives a task release
 * 
 * @param tcb Pointer to the TCB
 */
void sched_block(tcb_t *tcb);

/**
 * @brief Sets a task to wait for a message request
 * 
 * @param tcb Pointer to the TCB
 */
void sched_set_wait_request(tcb_t *tcb);

/**
 * @brief Sets a task to wait for a data av
 * 
 * @param tcb Pointer to the TCB
 */
void sched_set_wait_data_av(tcb_t *tcb);

/**
 * @brief Sets a task to wait for a message delivery
 * 
 * @param tcb Pointer to the TCB
 */
void sched_set_wait_delivery(tcb_t *tcb);

/**
 * @brief Gets the period constraint of a RT task
 * 
 * @param tcb Pointer to the TCB
 * 
 * @return Value of the period
 */
unsigned int sched_get_period(tcb_t *tcb);

/**
 * @brief Gets the deadline constraint of a RT task
 * 
 * @param tcb Pointer to the TCB
 * 
 * @return Value of the deadline
 */
int sched_get_deadline(tcb_t *tcb);

/**
 * @brief Gets the execution time constraint of a RT task
 * 
 * @param tcb Pointer to the TCB
 * 
 * @return Value of the execution time
 */
unsigned int sched_get_exec_time(tcb_t *tcb);

/**
 * @brief Sets the remaining execution time of a task
 * 
 * @param tcb Pointer to the TCB
 * @param timeslice Value to set to the remaining execution time
 */
void sched_set_remaining_time(tcb_t *tcb, unsigned int timeslice);

/**
 * @brief Runs the scheduler to set the current running task
 */
void sched_run();

/**
 * @brief Calls the scheduling algorithm
 * 
 * @details Selects the next real time task with the least slack time or the 
 * next best effort task following a round-robin order
 * 
 * @param current_time Current system time
 * 
 * @return Pointer to the scheduled TCB
 */
tcb_t *sched_lst(unsigned int current_time);

/**
 * @brief Runs the round robin for the scheduler
 * 
 * @return The next round robin iterator
 */
unsigned int sched_round_robin();

/**
 * @brief Updates the dynamic RT parameters for all RT tasks. 
 * 
 * @details The dynamic RT parameters are: remaining execution time, status and 
 * slack-time
 * 
 * @param current_time Current system time
 * @param schedule_overhead Dynamic calculated scheduling overhead time
 */
void sched_rt_update(unsigned int current_time, unsigned int schedule_overhead);

/**
 * @brief Updates the task slack time. 
 * 
 * @details The slack time is the time until the task starts the next period 
 * once it finishes its execution time for the current period
 * 
 * @param task Pointer to the TCB
 * @param current_time Current system time
 */
void sched_update_task_slack_time(tcb_t *task, unsigned int current_time);

/**
 * @brief This algorithm tries to give an extra time slice to task selected by LST.
 * 
 * @details The main approach is looking for the READY task slack time and the closer end of period
 * 
 * @param scheduled Pointer to the scheduled TCB
 * @param time Current system time
 */
void sched_dynamic_slice_time(tcb_t *scheduled, unsigned int time);

/**
 * @brief Sets real-time execution constraints
 * 
 * @param task Pointer to the TCB
 * @param period Task RT period
 * @param deadline Task RT deadline
 * @param execution_time Task RT execution time -- profile the task first
 */
void sched_real_time_task(tcb_t *task, unsigned int period, int deadline, unsigned int execution_time);

/**
 * @brief Gets the waiting message status
 * 
 * @param tcb Pointer to the TCB
 * 
 * @return The status of waiting message.
 */
unsigned sched_get_waiting_msg(tcb_t *tcb);

/**
 * @brief Sets the waiting message status
 * 
 * @param tcb Pointer to the TCB
 * @param waiting_msg Waiting message status
 */
void sched_set_waiting_msg(tcb_t *tcb, unsigned waiting_msg);
