/*!\file processors.c
 * MEMPHIS VERSION - 8.0 - support for RT applications
 *
 * Distribution:  June 2016
 *
 * Created by: Marcelo Ruaro - contact: marcelo.ruaro@acad.pucrs.br
 *
 * Research group: GAPH-PUCRS   -  contact:  fernando.moraes@pucrs.br
 *
 * \brief
 * This module implements function relative to the slave processor management by the manager kernel.
 * This modules is used only by the manager kernel
 */

#include "processors.h"
#include "utils.h"

Processor processors[MAX_CLUSTER_SLAVES];	//!<Processor array

/**Internal function to search by a processor - not visible to the other software part
 * \param proc_address Processor address to be searched
 * \return The processor pointer
 */
Processor * search_processor(int proc_address){

	for(int i=0; i<MAX_CLUSTER_SLAVES; i++){
		if (processors[i].address == proc_address){
			return &processors[i];
		}
	}

	putsv("ERROR: Processor not found ", proc_address);
	while(1);
	return 0;
}

/**Gets the processor address stored in the index parameter
 * \param index Index of the processor address
 * \return The processor address (XY)
 */
int get_proc_address(int index){

	return processors[index].address;
}

/**Updates the processor slack time
 * \param proc_address Processor address
 * \param slack_time Slack time in percentage
 */
void update_proc_slack_time(int proc_address, int slack_time){

	Processor * p = search_processor(proc_address);

	p->slack_time += slack_time;

	p->total_slack_samples++;

}

/**Gets the processor slack time
 * \param proc_address Processor address
 * \return The processor slack time in percentage
 */
int get_proc_slack_time(int proc_address){

	Processor * p = search_processor(proc_address);

	if (p->total_slack_samples == 0)
		return 100;

	return (p->slack_time/p->total_slack_samples);
}

/**Add a valid processor to the processors' array
 * \param proc_address Processor address to be added
 */
void add_procesor(int proc_address){

	Processor * p = search_processor(-1); //Searches for a free slot

	p->free_pages = MAX_LOCAL_TASKS;

	p->address = proc_address;

	p->slack_time = 0;

	p->total_slack_samples = 0;

	for(int i=0; i<MAX_LOCAL_TASKS; i++){
		p->task[i] = -1;
	}

}

/**Add a task into a processor. Called evenly when a task is mapped into a processor.
 * \param proc_address Processor address
 * \param task_id Task ID to be added
 */
void add_task(int proc_address, int task_id){

	Processor * p = search_processor(proc_address);

	//Index to add the task
	int add_i = -1;

	if (p->free_pages == 0){
		putsv("ERROR: Free pages is 0 - not add task enabled for proc", proc_address);
		while(1);
	}

	for(int i=0; i<MAX_LOCAL_TASKS; i++){
		if (p->task[i] == -1){
			add_i = i;
		}
		if (p->task[i] == task_id){
			putsv("ERROR: Task already added ", task_id);
			while(1);
		}
	}

	//add_i will be -1 when there is not more slots free into proc
	if (add_i > -1){
		p->task[add_i] = task_id;
		p->free_pages--;
	} else {
		putsv("ERROR: Not free slot into processor ", proc_address);
		while(1);
	}
}

/**Remove a task from a processor. Called evenly when a task is unmapped (removed) from a processor.
 * \param proc_address Processor address
 * \param task_id Task ID to be removed
 */
void remove_task(int proc_address, int task_id){

	Processor * p = search_processor(proc_address);

	if (p->free_pages == MAX_LOCAL_TASKS){
		putsv("ERROR: All pages free for proc", proc_address);
		while(1);
	}

	for(int i=0; i<MAX_LOCAL_TASKS; i++){
		if (p->task[i] == task_id){
			p->task[i] = -1;
			p->free_pages++;
			return;
		}
	}

	putsv("ERROR: Task not found to remove ", task_id);
	while(1);

}

/**Gets the total of free pages of a given processor
 * \param proc_address Processor address
 * \return The number of free pages
 */
int get_proc_free_pages(int proc_address){

	Processor * p = search_processor(proc_address);

	return p->free_pages;

}

/**Searches for a task location by walking for all processors within processors' array
 * \param task_id Task ID
 * \return The task location, i.e., the processor address that the task is allocated
 */
int get_task_location(int task_id){

	for(int i=0; i<MAX_CLUSTER_SLAVES; i++){

		if (processors[i].free_pages == MAX_LOCAL_TASKS || processors[i].address == -1){
			continue;
		}

		for(int t=0; t<MAX_LOCAL_TASKS; t++){

			if (processors[i].task[t] == task_id){

				return processors[i].address;
			}
		}
	}

	putsv("Warining: Task not found at any processor ", task_id);
	return -1;
}

/**Initializes the processors's array with invalid values
 */
void init_procesors(){
	for(int i=0; i<MAX_CLUSTER_SLAVES; i++){
		processors[i].address = -1;
		processors[i].free_pages = MAX_LOCAL_TASKS;
		processors[i].slack_time = 100;
		for(int t=0; t<MAX_LOCAL_TASKS; t++){
			processors[i].task[t] = -1;
		}
	}
}
