/*!\file task_location.c
 * MEMPHIS VERSION - 8.0 - support for RT applications
 *
 * Distribution:  June 2016
 *
 * Created by: Marcelo Ruaro - contact: marcelo.ruaro@acad.pucrs.br
 *
 * Research group: GAPH-PUCRS   -  contact:  fernando.moraes@pucrs.br
 *
 * \brief
 * This module implements function relative to task location structure. This module is used by the slave kernel
 * \detailed
 * The task location gives to the slave kernel, the location (slave process address) of the other task
 */

#include "task_location.h"
#include "utils.h"

TaskLocaion task_location[MAX_TASK_LOCATION];	//!<array of TaskLocation

/**Initializes task_location array with invalid values
 */
void init_task_location(){
	for(int i=0; i<MAX_TASK_LOCATION; i++) {
		task_location[i].id = -1;
		task_location[i].proc_address = -1;
	}
}

/**Searches for the location of a given task
 * \param task_ID The ID of the task
 * \return The task location (processor address in XY)
 */
int get_task_location(int task_ID){

	for(int i=0; i<MAX_TASK_LOCATION; i++) {
		if (task_location[i].id == task_ID){
			return task_location[i].proc_address;
		}
	}
	return -1;
}

/**Add a task_locaiton instance, removing, previously, any trash of task location for the same ID
 * \param task_ID Task ID
 * \param proc Location (address) of the task
 */
void add_task_location(int task_ID, int proc){

	//Ensure to remove all older instances of locations for this task_id
	for(int i=0; i<MAX_TASK_LOCATION; i++) {
		if (task_location[i].id == task_ID){
			task_location[i].id = -1;
			task_location[i].proc_address = -1;
		}
	}

	//Insert the task location in the first free position
	for(int i=0; i<MAX_TASK_LOCATION; i++) {
		if (task_location[i].id == -1){
			task_location[i].id = task_ID;
			task_location[i].proc_address = proc;
			//puts("Add task location - task id "); puts(itoa(task_ID)); puts(" proc "); puts(itoh(proc)); puts("\n");
			return;
		}
	}

	puts("ERROR - no FREE Task location\n");
	while(1);
}

/**Remove a task_locaton instance
 * \param task_id Task ID of the instance to be removed
 * \return The location of the removed task, -1 if ERROR
 */
int remove_task_location(int task_id){

	int r_proc;

	for(int i=0; i<MAX_TASK_LOCATION; i++) {

		if (task_location[i].id == task_id){

			task_location[i].id = -1;

			r_proc = task_location[i].proc_address;

			task_location[i].proc_address = -1;
			//puts("Add task location - task id "); puts(itoa(task_ID)); puts(" proc "); puts(itoh(proc)); puts("\n");
			return r_proc;
		}
	}

	puts("ERROR - task not found to remove\n");
	while(1);
	return -1;
}

/**Clear/remove all task of the same application
 * \param app_ID Application ID
 */
void clear_app_tasks_locations(int app_ID){

	for(int i=0;i<MAX_TASK_LOCATION; i++){
		if (task_location[i].id != -1 && (task_location[i].id >> 8) == app_ID){
			//puts("Remove task location - task id "); puts(itoa(task_location[i].id)); puts(" proc "); puts(itoh(task_location[i].proc_address)); puts("\n");
			task_location[i].id = -1;
			task_location[i].proc_address = -1;
		}
	}
}
