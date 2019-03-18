/*!\file applications.h
 * MEMPHIS VERSION - 8.0 - support for RT applications
 *
 * Distribution:  June 2016
 *
 * Created by: Marcelo Ruaro - contact: marcelo.ruaro@acad.pucrs.br
 *
 * Research group: GAPH-PUCRS   -  contact:  fernando.moraes@pucrs.br
 *
 * \brief Defines structures Application, Task, and Dependence.
 *
 * \detailed The Application structure is useful to the kernel_master to manage the applications' information.
 * This structure is also composed of the substructures: Task and Dependece.
 * Task stores the specific information of each applications's task
 * Dependence stores the task communication dependence information.
 */

#ifndef _APPLICATIONS_H_
#define _APPLICATIONS_H_
#include "../../include/kernel_pkg.h"

//Application status
#define	 RUNNING				1	//!< Signals that the application have all its task mapped and the task already has been requested
#define	 FREE					2	//!< Signals that all task of the application finishes
#define	 WAITING_RECLUSTERING	3	//!< Signals that the application have at least one task waiting for reclustering
#define	 READY_TO_LOAD			4	//!< Signals that the application have all its task mapped but the task not were requested yet

//Task status
#define	 REQUESTED				0	//!< Signals that the task has already requested to the global master
#define	 ALLOCATED				1	//!< Signals that the task has successfully allocated into a processor
#define	 TASK_RUNNING			2	//!< Signals that the task is running on the processor
#define	 TERMINATED_TASK		3	//!< Signals that the task terminated its execution
#define	 MIGRATING				4	//!< Signals that the task was set to be migrated on the slave kernel

#define MAX_TASK_DEPENDECES		10	//!< Stores maximum number of dependences task that a task can have

/**
 * \brief This structure stores the communication dependences of a given task
 * \detailed This structure is fulfilled by the kernel master during the reception of a new application.
 * Thus, the application repository keeps the dependence information for each task
 */
typedef struct {
	int task;		//!< Stores the communicating task ID
	int flits;		//!< Stores number of flits exchanged between the source task and the communicating task
} Dependence;

/**
 * \brief This structure stores variables useful to manage a task from a manager kernel perspective
 * \detailed Some of the values are loaded from repository others, changed at runtime
 */
typedef struct {
	int  id;						//!< Stores the task id
	int  code_size;					//!< Stores the task code size - loaded from repository
	int  data_size;					//!< Stores the DATA memory section size - loaded from repository
	int  bss_size;					//!< Stores the BSS memory section size - loaded from repository
	int  initial_address;			//!< Stores the initial address of task into repository - loaded from repository
	int  allocated_proc;			//!< Stores the allocated processor address of the task
	int  computation_load;			//!< Stores the computation load
	int  borrowed_master;			//!< Stores the borrowed master address
	int  status;					//!< Stores the status
	int  dependences_number;		//!< Stores number of communicating task, i.e, dependences
	Dependence dependences[10];		//!< Stores task dependence array with a size equal to MAX_TASK_DEPENDECES
} Task;

/** \brief This structure store variables useful to the kernel master manage an application instance
 */
typedef struct {
	int app_ID;					//!< Stores the application id
	int status;					//!< Stores the application status
	int tasks_number;			//!< Stores the application task number
	int terminated_tasks;		//!< Stores the number of task terminated of an application
	Task tasks[MAX_TASKS_APP];	//!< Array of Task with the size equal to the MAX_TASKS_APP
} Application;


Application * get_application_ptr(int);

Application * get_next_pending_app();

int set_task_allocated(Application *, int);

void set_task_terminated(Application *, int);

void set_task_migrating(int);

void set_task_migrated(int, int);

unsigned int get_app_id_counter();

Application * read_and_create_application(unsigned int, volatile unsigned int *);

void remove_application(int);

void initialize_applications();

#endif /* SOFTWARE_INCLUDE_APPLICATIONS_APPLICATIONS_H_ */
