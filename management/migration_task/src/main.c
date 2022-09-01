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
#include <memphis/services.h>

#include "migration.h"

#define MSG_LEN 3

int main()
{
	static int msg[MSG_LEN];

	printf("Migration started at %d\n", memphis_get_tick());
	
	size_t pe_cnt = memphis_get_nprocs(NULL, NULL);

	lru_t tasks;
	lru_init(&tasks, pe_cnt);

	oda_t actuator;
	oda_init(&actuator);
	oda_request_service(&actuator, ODA_ACT | A_MIGRATION);

	while(true){
		memphis_receive_any(msg, sizeof(msg));
		switch(msg[0]){
			case OBSERVE_PACKET:
				// Echo("Hello, received observe packet\n");
				mt_check_rt(&tasks, &actuator, msg[1], msg[2]);
				break;
			case SERVICE_PROVIDER:
				oda_service_provider(&actuator, msg[1], msg[2]);
				break;
		}
	}

	return 0;
}
