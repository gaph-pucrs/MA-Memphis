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
 * @brief Main file of the MA-Memphis mapper
 */

#include <stdio.h>
#include <stdlib.h>
#include <memphis.h>
#include <errno.h>

#include "services.h"
#include "task_migration.h"

int main()
{
	// printf("OI\n");
	// while(1);

	unsigned *a = malloc(sizeof(unsigned)*1564);
	if(a == NULL){
		printf("E=%d\n", errno);
		while(1);
	} else {
		a[1563] = memphis_get_tick();
		printf("ADDR=%d    T=%u\n", a, a[1563]);
		while(1);
	}

	
	// printf("Mapper task started at time %u\n", memphis_get_tick());

	static mapper_t mapper;	
	map_init(&mapper);

	while(true){
		static message_t msg;
		memphis_receive_any(&msg);
		/* Check what service has been received */
		switch(msg.payload[0]){
			case NEW_APP:
				map_new_app(&mapper, msg.payload[1], (int*)&msg.payload[2], (int*)&msg.payload[2*msg.payload[1] + 2]);
				break;
			case TASK_ALLOCATED:
				map_task_allocated(&mapper, msg.payload[1]);
				break;
			case TASK_TERMINATED:
				map_task_terminated(&mapper, msg.payload[1]);
				break;
			case TASK_MIGRATION_MAP:
				tm_migrate(&mapper, msg.payload[1]);
				break;
			case TASK_MIGRATED:
				tm_migration_complete(&mapper, msg.payload[1]);
				break;
			case REQUEST_SERVICE:
				map_request_service(&mapper, msg.payload[1], msg.payload[2], msg.payload[3]);
				break;
			case TASK_ABORTED:
				map_task_aborted(&mapper, msg.payload[1]);
				break;
			default:
				printf("Invalid service %d received\n", msg.payload[0]);
				break;
		}
	}
}
