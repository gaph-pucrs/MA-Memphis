/**
 * MA-Memphis
 * @file main.c
 *
 * @author Angelo Elias Dalzotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date March 2021
 * 
 * @brief Main migration decider file
 */

#include <stdio.h>
#include <memphis.h>

#include "migration.h"
#include "services.h"
#include "tag.h"

int main()
{
	static message_t msg;

	printf("Migration started at %d\n", memphis_get_tick());
	
	migration_ring_t decider;
	migration_init(&decider);

	oda_t actuator;
	oda_init(&actuator);
	oda_request_service(&actuator, ODA_ACT | A_MIGRATION);

	while(true){
		memphis_receive_any(&msg);
		switch(msg.payload[0]){
		case OBSERVE_PACKET:
			// Echo("Hello, received observe packet\n");
			migration_check_rt(&decider, &actuator, msg.payload[1], msg.payload[2]);
			break;
		case SERVICE_PROVIDER:
			oda_service_provider(&actuator, msg.payload[1], msg.payload[2]);
			break;
		}
	}

	return 0;
}
