/*!\file task_migration.h
 * MEMPHIS VERSION - 8.0 - support for RT applications
 *
 * Distribution:  June 2016
 *
 * Created by: Marcelo Ruaro - contact: marcelo.ruaro@acad.pucrs.br
 *
 * Research group: GAPH-PUCRS   -  contact:  fernando.moraes@pucrs.br
 *
 * \brief This module defines function relative to task migration.
 *
 */
#ifndef SOFTWARE_INCLUDE_TASK_MIGRATION_H_
#define SOFTWARE_INCLUDE_TASK_MIGRATION_H_

#include "../../include/kernel_pkg.h"
#include "packet.h"
#include "task_control.h"


int handle_migration(volatile ServiceHeader *, unsigned int);

void migrate_dynamic_memory(TCB *);


#endif /* SOFTWARE_INCLUDE_TASK_MIGRATION_H_ */
