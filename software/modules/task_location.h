/*!\file task_location.h
 * MEMPHIS VERSION - 8.0 - support for RT applications
 *
 * Distribution:  June 2016
 *
 * Created by: Marcelo Ruaro - contact: marcelo.ruaro@acad.pucrs.br
 *
 * Research group: GAPH-PUCRS   -  contact:  fernando.moraes@pucrs.br
 *
 * \brief
 * This module defines function relative to task location structure.
 * \detailed
 * The TaskLocation structure is defined, this structure stores the location (slave process address) of the other task
 *
 */

#ifndef TASK_LOCATION_H_
#define TASK_LOCATION_H_

#include "../../include/kernel_pkg.h"

#define MAX_TASK_LOCATION	(MAX_LOCAL_TASKS*MAX_TASKS_APP)

/**
 * \brief This structure stores the location (slave process address) of the other task
 */
typedef struct {
	int id;				//!<ID of task
	int proc_address;	//!<processor address of the task
} TaskLocaion;

void init_task_location();

int get_task_location(int);

void add_task_location(int, int);

int remove_task_location(int);

void clear_app_tasks_locations(int);



#endif /* TASK_LOCATION_H_ */
