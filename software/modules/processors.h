/*!\file processors.h
 * MEMPHIS VERSION - 8.0 - support for RT applications
 *
 * Distribution:  June 2016
 *
 * Created by: Marcelo Ruaro - contact: marcelo.ruaro@acad.pucrs.br
 *
 * Research group: GAPH-PUCRS   -  contact:  fernando.moraes@pucrs.br
 *
 * \brief
 * This module defines function relative to the slave processor management by the manager kernel.
 * \detailed
 * The Processor structure is defined, this structure stores information relative the slave processor, which are used by the
 * kernel master to search task locations and get the number of free pages of each slave during the mapping processes
 */

#ifndef PROCESSOR_H_
#define PROCESSOR_H_

#include "../../include/kernel_pkg.h"

/**
 * \brief This structure store variables used to manage the processors attributed by the kernel master
 */
typedef struct {
	int address;						//!<Processor address in XY
	int free_pages;						//!<Number of free memory pages
	int slack_time;						//!<Slack time (idle time), represented in percentage
	unsigned int total_slack_samples;	//!<Number of slack time samples
	int task[MAX_LOCAL_TASKS]; 			//!<Array with the ID of all task allocated in a given processor
} Processor;

void init_procesors();

void update_proc_slack_time(int, int);

int get_proc_slack_time(int);

void add_procesor(int);

void add_task(int, int);

void remove_task(int, int);

int get_proc_free_pages(int);

int get_proc_address(int);

int get_task_location(int);



#endif /* PROCESSOR_H_ */
