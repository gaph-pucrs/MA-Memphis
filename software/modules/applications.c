/*!\file applications.c
 * MEMPHIS VERSION - 8.0 - support for RT applications
 *
 * Distribution:  June 2016
 *
 * Created by: Marcelo Ruaro - contact: marcelo.ruaro@acad.pucrs.br
 *
 * Research group: GAPH-PUCRS   -  contact:  fernando.moraes@pucrs.br
 *
 * \brief
 * Implements the function to manage a insertion, accessing, and remotion of an applications structure
 * This modules is only used by manager kernel
 */

#include "applications.h"
#include "utils.h"

//Globals
Application applications[MAX_CLUSTER_APP];	//!< Store the applications informations, is equivalent to a attribute in OO paradigm


/** Receives and app ID and return the Application pointer for the required app ID.
 * If not found, the kernel entering in a error situation
 *  \param app_id ID of the application
 *  \return Pointer for Application
 */
Application * get_application_ptr(int app_id){

	for(int i=0; i<MAX_CLUSTER_APP; i++){
		if (applications[i].app_ID == app_id){
			return &applications[i];
		}
	}

	putsv("ERROR: Applications not found ", app_id);
	while(1);
	return 0;
}

/** Searches for a Task pointer into an Application instance. If not found, the kernel entering in a error situation
 *  \param app Wanted Application pointer
 *  \return Task pointer
 */
Task * get_task_ptr(Application * app, int task_id){

	for(int i=0; i<app->tasks_number; i++){

		if (app->tasks[i].id == task_id){

			return &app->tasks[i];

		}
	}

	putsv("ERROR: Task not found ", task_id);
	while(1);
	return 0;
}


/** Get the oldest application waiting reclustering
 *  \return Selected Application pointer
 */
Application * get_next_pending_app(){

	Application * older_app = 0;

	for(int i=0; i<MAX_CLUSTER_APP; i++){
		if (applications[i].status == WAITING_RECLUSTERING){

			if (older_app == 0 || older_app->app_ID > applications[i].app_ID){
				older_app = &applications[i];
			}
		}
	}

	if (!older_app){
		puts("ERROR: no one app waiting reclustering\n");
		while(1);
	}

	return older_app;
}

/** Set a task status as allocated and verifies the number of allocated task for the application
 * \param app Application pointer of the task
 * \param task_id ID of the allocated task
 * \return The number of allocated task for the application
 */
int set_task_allocated(Application * app, int task_id){

	int task_allocated = 0;

	for(int i=0; i<app->tasks_number; i++){

		if (app->tasks[i].id == task_id){

			app->tasks[i].status = ALLOCATED;

			task_allocated++;

		} else if ( app->tasks[i].status == ALLOCATED){
			task_allocated++;
		}
	}

	return task_allocated;
}

/** Set a task status as terminated
 * \param app Application pointer of the task
 * \param task_id ID of the terminated task
 */
void set_task_terminated(Application * app, int task_id){

	Task * t_ptr;

	app->terminated_tasks++;

	t_ptr = get_task_ptr(app, task_id);

	t_ptr->status = TERMINATED_TASK;
}

/** Set a task status as migrating
 * \param app Application pointer of the task
 * \param task_id ID of the migrating task
 */
void set_task_migrating(int task_id){

	Task * t_ptr;

	Application * app = get_application_ptr( (task_id >> 8) );

	t_ptr = get_task_ptr(app, task_id);

	t_ptr->status = MIGRATING;
}

/** Set a task status as migrated
 * \param app Application pointer of the task
 * \param task_id ID of the migrated task
 */
void set_task_migrated(int task_id, int new_proc){

	Task * t_ptr;

	Application * app = get_application_ptr( (task_id >> 8) );

	t_ptr = get_task_ptr(app, task_id);

	t_ptr->status = TASK_RUNNING;

	t_ptr->allocated_proc = new_proc;
}



Application * read_and_create_application(unsigned int app_id, volatile unsigned int * ref_address){

	volatile unsigned int task_id;
	Task * tp;
	Application * app = get_application_ptr(-1);

	app->app_ID = app_id;

	app->tasks_number = *(ref_address++);

	for(int k=0; k < app->tasks_number; k++){

		task_id = *(ref_address++);
		task_id = task_id & (unsigned int) 0xFF;

		tp = &app->tasks[task_id];
		tp->allocated_proc = *(ref_address++);
		tp->code_size = *(ref_address++);
		tp->data_size = *(ref_address++);
		tp->bss_size = *(ref_address++);
		tp->initial_address = *(ref_address++);
		tp->dependences_number = 0;
		tp->computation_load = 0;
		tp->id = app_id << 8 | task_id;
		tp->borrowed_master = -1;

		putsv("Creating task: ", tp->id);
		putsv("	code_size: ", tp->code_size);
		putsv("	initial_address: ", tp->initial_address);
		putsv("	allocated proc: : ", tp->allocated_proc);

		/*for(int j=0; j < MAX_TASK_DEPENDECES; j++){

			tp->dependences[j].task = (app_id << 8) | *(ref_address++);
			tp->dependences[j].flits = *(ref_address++);

			if(tp->dependences[j].task != -1) {
				tp->dependences_number++;
			}
		}*/
	}

	return app;
}

void remove_application(int app_id){

	Application * app = get_application_ptr(app_id);

	app->app_ID = -1;

	app->status = FREE;

	app->tasks_number = 0;

	app->terminated_tasks = 0;
}

void initialize_applications(){

	for(int i=0; i<MAX_CLUSTER_APP; i++){

		applications[i].app_ID = -1;

		applications[i].tasks_number = 0;

		applications[i].status = FREE;

		applications[i].terminated_tasks = 0;
	}
}

