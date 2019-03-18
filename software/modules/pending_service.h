/*!\file pending_service.h
 * MEMPHIS VERSION - 8.0 - support for RT applications
 *
 * Distribution:  June 2016
 *
 * Created by: Marcelo Ruaro - contact: marcelo.ruaro@acad.pucrs.br
 *
 * Research group: GAPH-PUCRS   -  contact:  fernando.moraes@pucrs.br
 *
 * \brief
 *  This module defines function relative a FIFO of the incomming packets received by slave by which not can be
 * immediately handled. This modules is used only by the slave kernel
 */

#ifndef SOFTWARE_INCLUDE_PENDING_SERVICE_PENDING_SERVICE_H_
#define SOFTWARE_INCLUDE_PENDING_SERVICE_PENDING_SERVICE_H_
#include "../../include/kernel_pkg.h"
#include "packet.h"

#define PENDING_SERVICE_TAM		20		//!<Pending service array size

unsigned char add_pending_service(ServiceHeader *);

ServiceHeader * get_next_pending_service();

#endif /* SOFTWARE_INCLUDE_PENDING_SERVICE_PENDING_SERVICE_H_ */
