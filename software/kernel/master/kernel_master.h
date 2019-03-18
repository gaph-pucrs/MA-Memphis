/*!\file kernel_master.h
 * MEMPHIS VERSION - 8.0 - support for RT applications
 *
 * Distribution:  June 2016
 *
 * Created by: Marcelo Ruaro - contact: marcelo.ruaro@acad.pucrs.br
 *
 * Research group: GAPH-PUCRS   -  contact:  fernando.moraes@pucrs.br
 *
 * \brief
 * Header of kernel_master with important defines
 */

#ifndef __KERNEL_MASTER_H__
#define __KERNEL_MASTER_H__

#include "../../../include/kernel_pkg.h"

#define TASK_DESCRIPTOR_SIZE	26	//!< Size of the task descriptor into repository.txt file

/* Useful macros */
#define	noc_interruption 		(MemoryRead(IRQ_STATUS) & IRQ_NOC)	//!< Signals a incoming packet from NoC
#define get_cluster_proc(x)		( (cluster_info[x].master_x << 8) | cluster_info[x].master_y)
#define net_address				( (cluster_info[clusterID].master_x << 8) | cluster_info[clusterID].master_y)

//These functions are externed only for remove warnings into kernel_master.c code
void handle_new_app(int, volatile unsigned int *, unsigned int);
void initialize_slaves();

#endif
