/*!\file reclustering.h
 * MEMPHIS VERSION - 8.0 - support for RT applications
 *
 * Distribution:  June 2016
 *
 * Created by: Marcelo Ruaro - contact: marcelo.ruaro@acad.pucrs.br
 *
 * Research group: GAPH-PUCRS   -  contact:  fernando.moraes@pucrs.br
 *
 * \breif
 * This module defines function relative to reclustering
 * \detailed
 * The Reclustering structure is defined, this structure contains information
 * relative to the manager controls the reclustering process.
 */
#ifndef SOFTWARE_INCLUDE_RECLUSTERING_RECLUSTERING_H_
#define SOFTWARE_INCLUDE_RECLUSTERING_RECLUSTERING_H_

#include "../../include/kernel_pkg.h"
#include "applications.h"
#include "packet.h"

typedef struct {
	Task * task;				//!<Task pointer of the task under reclustering
	int active;					//!<Reclustering status (active 1, or disabled 0)
	int pending_loan_delivery;	//!<Number of pending load delivery which the master is waiting
	int min_loan_proc_hops;		//!<Minimum number of hops from borrowed processor
	int current_borrowed_proc;	//!<Borrowed processor address
	int current_borrowed_master;//!<Borrowed processor master address
	int neighbors_level;		//!<Number of neighbors levels (clusters levels) used in the reclustering protocol
} Reclustering;

extern unsigned int clusterID; //!<Cluster ID index of array cluster_info - clusterID is the only extern global variable in in all software


int is_reclustering_NOT_active();

void reclustering_setup(int);

void handle_reclustering(ServiceHeader *);

int reclustering_next_task(Application *);


#endif /* SOFTWARE_INCLUDE_RECLUSTERING_RECLUSTERING_H_ */
