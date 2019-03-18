/*!\file task_control.c
 * MEMPHIS VERSION - 8.0 - support for RT applications
 *
 * Distribution:  June 2016
 *
 * Created by: Marcelo Ruaro - contact: marcelo.ruaro@acad.pucrs.br
 *
 * Research group: GAPH-PUCRS   -  contact:  fernando.moraes@pucrs.br
 *
 * \brief
 * This module implements function relative to task control block (TCB)
 * This module is used by the slave kernel
 */

#include "task_control.h"

#include "../../include/kernel_pkg.h"
#include "utils.h"


TCB tcbs[MAX_LOCAL_TASKS];			//!<Local task TCB array

/**Initializes TCB array
 */
void init_TCBs(){


	for(int i=0; i<MAX_LOCAL_TASKS; i++) {
		tcbs[i].id = -1;
		tcbs[i].pc = 0;
		tcbs[i].offset = PAGE_SIZE * (i + 1);
		tcbs[i].proc_to_migrate = -1;

		init_scheduling_ptr(&tcbs[i].scheduling_ptr, i);

		tcbs[i].scheduling_ptr->tcb_ptr = (unsigned int) &tcbs[i];

		clear_scheduling( tcbs[i].scheduling_ptr );

	}
}

/**Search from a tcb position with status equal to FREE
 * \return The TCB pointer or 0 in a ERROR situation
 */
TCB* search_free_TCB() {

    for(int i=0; i<MAX_LOCAL_TASKS; i++){
		if(tcbs[i].scheduling_ptr->status == FREE){
			return &tcbs[i];
		}
	}

    puts("ERROR - no FREE TCB\n");
    while(1);
    return 0;
}

/**Search by a TCB
 * \param task_id Task ID to be searched
 * \return TCB pointer
 */
TCB * searchTCB(unsigned int task_id) {

    int i;

    for(i=0; i<MAX_LOCAL_TASKS; i++)
    	if(tcbs[i].id == task_id)
    		return &tcbs[i];

    return 0;
}

/**Gets the TCB pointer from a index
 * \param i Index of TCB
 * \return The respective TCB pointer
 */
TCB * get_tcb_index_ptr(unsigned int i){
	return &(tcbs[i]);
}

/**Test if there is another task of the same application running in the same slave processor
 * \param app_id Appliation ID
 * \return 1 - if YES, 0 if NO
 */
int is_another_task_running(int app_id){

	for (int i = 0; i < MAX_LOCAL_TASKS; i++){
		if (tcbs[i].scheduling_ptr->status != FREE && (tcbs[i].id >> 8) == app_id){
			return 1;
		}
	}
	return 0;
}


