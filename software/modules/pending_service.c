/*!\file pending_service.c
 * MEMPHIS VERSION - 8.0 - support for RT applications
 *
 * Distribution:  June 2016
 *
 * Created by: Marcelo Ruaro - contact: marcelo.ruaro@acad.pucrs.br
 *
 * Research group: GAPH-PUCRS   -  contact:  fernando.moraes@pucrs.br
 *
 * \brief
 * This module implements function relative a FIFO of the incomming packets (ServiceHeader FIFO) received by slave and that cannot be
 * immediately handled.
 * This modules is used only by the slave kernel
 */


#include "pending_service.h"
#include "../include/plasma.h"
#include "utils.h"

ServiceHeader pending_services_FIFO[PENDING_SERVICE_TAM];	//!<pending services array declaration

unsigned int pending_service_first = 0;	//!<first valid array index
unsigned int pending_service_last = 0;	//!<last valid array index

unsigned char add_fifo = 0; 	//!<Keeps the last operation: 1 - last operation was add. 0 - last operation was remove


/**Add a new pending service. A pending service is a incoming service that cannot be handled immediately by kernel
 * \param pending_service Incoming ServiceHeader pointer
 * \return 1 if add was OK, 0 is the array is full (ERROR situation, while forever)
 */
unsigned char add_pending_service(ServiceHeader * pending_service){

	ServiceHeader * fifo_free_position;

	//Test if the buffer is full
	if (pending_service_first == pending_service_last && add_fifo == 1){
		puts("ERROR: Pending service FIFO FULL\n");
		while(1);
		return 0;
	}

	fifo_free_position = &pending_services_FIFO[pending_service_last];

	//equivalent to a memcopy once both are the same struct
	*fifo_free_position = *pending_service;

	if (pending_service_last == PENDING_SERVICE_TAM-1){
		pending_service_last = 0;
	} else {
		pending_service_last++;
	}

	add_fifo = 1;

	//puts("Interruption set ON\n");
	MemoryWrite(PENDING_SERVICE_INTR, 1);

	return 1;
}

/**Searches by the next pending service, remove then, and return
 * \return The removed ServiceHeader pointer. As it is a memory position, the received part will do something with this pointer and discard the
 * reference
 */
ServiceHeader * get_next_pending_service(){

	ServiceHeader *service_header_to_ret;

	//Test if the buffer is empty
	if (pending_service_first == pending_service_last && add_fifo == 0){
		return 0;
	}

	service_header_to_ret = &pending_services_FIFO[pending_service_first];

	if (pending_service_first == PENDING_SERVICE_TAM-1){
		pending_service_first = 0;
	} else {
		pending_service_first++;
	}

	add_fifo = 0;

	//Test if the buffer is empty
	if (pending_service_first == pending_service_last){
		//puts("Interruption set OFF\n");
		MemoryWrite(PENDING_SERVICE_INTR, 0);
	}

	return service_header_to_ret;
}
