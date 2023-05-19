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

/* Forward declaration */
typedef struct _tcb tcb_t;

/**
 * @brief Reasons the task can be blocked for a message synchronization.
 */
typedef enum _sched_wait {
	SCHED_WAIT_NO,
	SCHED_WAIT_DELIVERY,
	SCHED_WAIT_DATA_AV,
	SCHED_WAIT_REQUEST
} sched_wait_t;

/**
 * @brief Task scheduling status.
 */
typedef enum _sched_status {
	SCHED_READY,	//!< Task is in ready to execute
	SCHED_RUNNING,	//!< Task is running in the CPU
	SCHED_SLEEPING	//!< Task already executed all its execution time and is sleeping until the end of period
} sched_status_t;

/**
 * @brief This structure stores variables useful to manage the task scheduling 
 * (for BE or RT).
 * 
 * @details Some of the values are only used for RT tasks
 */
typedef struct _sched {
	tcb_t *tcb;	//<! Pointer to the TCB of the scheduling structure

	unsigned exec_time;			//!< Task execution time in clock cycles
	unsigned period;			//!< Task period in clock cycles
	int		 deadline;			//!< Task deadline in clock cycles, for BE task is set to -1
	unsigned last_monitored;	//!< Last tick that the RT task was monitored

	unsigned ready_time;			//!< Time in clock cycles that task becomes ready
	int		 remaining_exec_time;	//!< Task remaining execution time in clock cycles
	unsigned slack_time;			//!< Task slack time in clock cycles
	unsigned running_start_time;	//!< Task running start time in clock cycles
	unsigned utilization;			//!< Task CPU utilization in percentage

	sched_status_t status;		//!< Task scheduling status
	sched_wait_t waiting_msg;	//!< Signals when task is waiting a message from a producer task
} sched_t;

/**
 * @brief Initializes the scheduler for the first time.
 */
void sched_init();

/**
 * @brief Creates and inserts a scheduler into list
 * 
 * @param tcb Pointer to the TCB to associate with the scheduler
 * @return sched_t* Pointer to the scheduler
 */
sched_t *sched_emplace_back(tcb_t *tcb);

/**
 * @brief Checks if the scheduler is idle.
 * 
 * @return True if scheduled task is the idle one.
 */
bool sched_is_idle();

/**
 * @brief Removes a scheduler from the list
 * 
 * @param sched Pointer to the scheduler
 */
void sched_remove(sched_t *sched);

/**
 * @brief Get the current scheduled task.
 * 
 * @return Pointer to the TCB of the scheduled task.
 */
tcb_t *sched_get_current_tcb();

/**
 * @brief Updates the total slack time.
 */
void sched_update_slack_time();

/**
 * @brief Checks if the task is waiting for a MESSAGE_REQUEST
 * 
 * @param sched Pointer to the scheduler
 * 
 * @return True if the task is waiting
 */
bool sched_is_waiting_msgreq(sched_t *sched);

/**
 * @brief Checks if the task is waiting for a DATA_AV
 * 
 * @param sched Pointer to the scheduler
 * 
 * @return True if the task is waiting
 */
bool sched_is_waiting_dav(sched_t *sched);

/**
 * @brief Checks if the task is waiting for a message delivery
 * 
 * @param sched Pointer to the scheduler
 * 
 * @return True if the task is waiting
 */
bool sched_is_waiting_delivery(sched_t *sched);

/**
 * @brief Releases the task from a waiting message condition
 * 
 * @param sched Pointer to the scheduler
 */
void sched_release_wait(sched_t *sched);

/**
 * @brief Updates the scheduler idle counter
 */
void sched_update_idle_time();

/**
 * @brief Sets a task to wait for a message request
 * 
 * @param sched Pointer to the scheduler
 */
void sched_set_wait_msgreq(sched_t *sched);

/**
 * @brief Sets a task to wait for a data av
 * 
 * @param sched Pointer to the scheduler
 */
void sched_set_wait_dav(sched_t *sched);

/**
 * @brief Sets a task to wait for a message delivery
 * 
 * @param sched Pointer to the scheduler
 */
void sched_set_wait_msgdlvr(sched_t *sched);

/**
 * @brief Gets the period constraint of a RT task
 * 
 * @param sched Pointer to the scheduler
 * 
 * @return Value of the period
 */
unsigned sched_get_period(sched_t *sched);

/**
 * @brief Gets the deadline constraint of a RT task
 * 
 * @param sched Pointer to the scheduler
 * 
 * @return Value of the deadline
 */
int sched_get_deadline(sched_t *sched);

/**
 * @brief Gets the execution time constraint of a RT task
 * 
 * @param sched Pointer to the scheduler
 * 
 * @return Value of the execution time
 */
unsigned sched_get_exec_time(sched_t *sched);

/**
 * @brief Runs the scheduler to set the current running task
 */
void sched_run();

/**
 * @brief Sets real-time execution constraints
 * 
 * @param sched Pointer to the scheduler
 * @param period Task RT period
 * @param deadline Task RT deadline
 * @param execution_time Task RT execution time -- profile the task first
 */
void sched_real_time_task(sched_t *sched, unsigned period, int deadline, unsigned execution_time);

/**
 * @brief Gets the waiting message status
 * 
 * @param sched Pointer to the scheduler
 * 
 * @return sched_wait_t The status of waiting message.
 */
sched_wait_t sched_get_waiting_msg(sched_t *sched);

/**
 * @brief Sets the waiting message status
 * 
 * @param tcb Pointer to the TCB
 * @param waiting_msg Waiting message status
 */
void sched_set_waiting_msg(sched_t *sched, sched_wait_t waiting_msg);

/**
 * @brief Reports an interruption
 */
void sched_report_interruption();

/**
 * @brief Report a scheduled task
 * 
 * @param id ID of the scheduled task
 */
void sched_report(int id);
