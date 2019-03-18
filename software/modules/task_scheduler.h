/*!\file task_scheduler.h
 * MEMPHIS VERSION - 8.0 - support for RT applications
 *
 * Distribution:  June 2016
 *
 * Created by: Marcelo Ruaro - contact: marcelo.ruaro@acad.pucrs.br
 *
 * Research group: GAPH-PUCRS   -  contact:  fernando.moraes@pucrs.br
 *
 * \brief Defines Scheduling structure.
 * \detailed
 * This struct is used to manages the scheduling informations relative to each task
 *
 * A given task can be BE (Best-Effort) or RT (Real-Time).
 *
 * BE task have not timing constraints.
 * RT have the timing constraints defined according the next diagram
 *
 * Periodic task real-time parameters
 *
 * |<------------ period -----------------> |
 * |										|
 * |<------------deadline------------>|	    |
 * |							      | 	|
 * |            		  		      |		|
 * | _______________________          |		|
 * || 	execution time		|<-slack->|		|
 * ||_______________________|_________|_____|
 * ^			time ->
 * |
 * |
 * ready time
 */

#ifndef TASK_SCHEDULER_H_
#define TASK_SCHEDULER_H_

//Task scheduling status
#define FREE  			-1		//!<Scheduling position not have a valid task
#define READY       	0		//!<Task is in ready to execute
//#define WAITING     	1		//!<Task is waiting for a message
#define RUNNING     	2		//!<Task is running in the CPU
#define MIGRATING		3		//!<Task is being migrated
#define BLOCKED			4		//!<Task is blocked waiting for the manager send a TASK_RELEASE packet
#define SLEEPING	  	5		//!<Task already executed all its execution time and is sleeping until the end of period


//Auxiliary RT defines
#define DEBUG					0			//!<When enabled shows the puts placed within local_scheduler.c
#define NO_DEADLINE				-1			//!<A task that is best-effor have its deadline variable equal to -1
#define MAX_TIME_SLICE			16318		//!<Standard time slice value for task execution

/**
 * \brief This structure stores variables useful to manage the task scheduling (for BE or RT)
 * \detailed Some of the values are only used for RT tasks
 */
typedef struct {

	int 			status;				//!<Task scheduling status
	unsigned int 	execution_time;		//!<Task execution time in clock cycles
	unsigned int 	period;				//!<Task period in clock cycles
	int 			deadline;			//!<Task deadline in clock cycles, for BE task is set to -1
	unsigned int 	ready_time;			//!<Time in clock cycles that task becomes ready
	int 			remaining_exec_time;//!<Task remaining execution time in clock cycles
	unsigned int 	slack_time;			//!<Task slack time in clock cycles
	unsigned int 	running_start_time;	//!<Task running start time in clock cycles
	unsigned int 	utilization;		//!<Task CPU utilization in percentage
	unsigned int 	waiting_msg;		//!<Signals when task is waiting a message from a producer task

	unsigned int 	tcb_ptr;			//!<Pointer for the respective task TCB into kernel slave

} Scheduling;


void real_time_task(Scheduling *, unsigned int, int, unsigned int);

unsigned int get_time_slice();

void init_scheduling_ptr(Scheduling **, int);

void clear_scheduling(Scheduling *);

Scheduling * LST(unsigned int);


#endif /* TASK_SCHEDULER_H_ */
