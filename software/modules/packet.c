/*!\file packet.c
 * MEMPHIS VERSION - 8.0 - support for RT applications
 *
 * Distribution:  June 2016
 *
 * Created by: Marcelo Ruaro - contact: marcelo.ruaro@acad.pucrs.br
 *
 * Research group: GAPH-PUCRS   -  contact:  fernando.moraes@pucrs.br
 *
 * \brief
 * This module implements function relative to programming the DMNI to send and receibe a packet.
 * \detailed
 * It is a abstraction from the NoC to the software components.
 * This module is used by both manager and slave kernel
 */

#include "packet.h"
#include "../include/plasma.h"

ServiceHeaderSlot sh_slot1, sh_slot2;	//!<Slots to prevent memory writing while is sending a packet

unsigned int global_inst = 0;			//!<Global CPU instructions counter


/**Searches for a free ServiceHeaderSlot (sh_slot1 or sh_slot2) pointer.
 * A free slot is the one which is not being used by DMNI. This function prevents that
 * a given memory space be changed while its is not completely transmitted by DMNI.
 * \return A pointer to a free ServiceHeadeSlot
 */
ServiceHeader* get_service_header_slot() {

	if ( sh_slot1.status ) {

		sh_slot1.status = 0;
		sh_slot2.status = 1;
		return &sh_slot1.service_header;

	} else {

		sh_slot2.status = 0;
		sh_slot1.status = 1;
		return &sh_slot2.service_header;
	}
}

/**Initializes the service slots
 */
void init_service_header_slots(){
	sh_slot1.status = 1;
	sh_slot2.status = 1;
}

/**Function that abstracts the DMNI programming for read data from NoC and copy to memory
 * \param initial_address Initial memory address to copy the received data
 * \param dmni_msg_size Data size, is represented in memory word of 32 bits
 */
void DMNI_read_data(unsigned int initial_address, unsigned int dmni_msg_size){

	MemoryWrite(DMNI_SIZE, dmni_msg_size);
	MemoryWrite(DMNI_OP, WRITE);
	MemoryWrite(DMNI_ADDRESS, initial_address);
	MemoryWrite(DMNI_START, 1);
	while (MemoryRead(DMNI_RECEIVE_ACTIVE));
}

/**Function that abstracts the DMNI programming for send data from memory to NoC
 * \param initial_address Initial memory address that will be transmitted to NoC
 * \param dmni_msg_size Data size, is represented in memory word of 32 bits
 */
void DMNI_send_data(unsigned int initial_address, unsigned int dmni_msg_size){

	while (MemoryRead(DMNI_SEND_ACTIVE));

	MemoryWrite(DMNI_SIZE, dmni_msg_size);
	MemoryWrite(DMNI_OP, READ);
	MemoryWrite(DMNI_ADDRESS, initial_address);
	MemoryWrite(DMNI_START, 1);
}

/**Function that abstracts the process to send a generic packet to NoC by programming the DMNI
 * \param p Packet pointer
 * \param initial_address Initial memory address of the packet payload (payload, not service header)
 * \return dmni_msg_size Packet payload size represented in memory words of 32 bits
 */
void send_packet(ServiceHeader *p, unsigned int initial_address, unsigned int dmni_msg_size){

	p->payload_size = (CONSTANT_PKT_SIZE - 2) + dmni_msg_size;

	p->transaction = 0;

	p->source_PE = MemoryRead(NI_CONFIG);

	//Waits the DMNI send process be released
	while (MemoryRead(DMNI_SEND_ACTIVE));

	p->timestamp = MemoryRead(TICK_COUNTER);

	MemoryWrite(DMNI_SIZE, CONSTANT_PKT_SIZE);
	MemoryWrite(DMNI_ADDRESS, (unsigned int) p);

	if (dmni_msg_size > 0){

		MemoryWrite(DMNI_SIZE_2, dmni_msg_size);
		MemoryWrite(DMNI_ADDRESS_2, initial_address);
	}

	MemoryWrite(DMNI_OP, READ);
	MemoryWrite(DMNI_START, 1);

}

/**Function that abstracts the process to read a generic packet from NoC by programming the DMNI
 * \param p Packet pointer
 */
void read_packet(ServiceHeader *p){

	MemoryWrite(DMNI_SIZE, CONSTANT_PKT_SIZE);
	MemoryWrite(DMNI_OP, WRITE);
	MemoryWrite(DMNI_ADDRESS, (unsigned int) p);
	MemoryWrite(DMNI_START, 1);
	//Waits the DMNI copy all data to memory before release the software to access it
	while (MemoryRead(DMNI_RECEIVE_ACTIVE));

}
