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

#include <errno.h>

#include <memphis.h>
#include <memphis/services.h>

#include "task_migration.h"

const size_t MAX_SIZE = 128;

int main()
{
	printf("Mapper task started at time %u\n", memphis_get_tick());

	static mapper_t mapper;	
	map_init(&mapper);

	size_t alloc_size = MAX_SIZE*sizeof(int);
	int *in_msg = malloc(alloc_size);

	if(in_msg == NULL){
		puts("Not enough memory");
		return errno;
	}

	while(true){
		memphis_receive_any(in_msg, alloc_size);
		/* Check what service has been received */
		switch(in_msg[0]){
			case NEW_APP:
				map_new_app(&mapper, in_msg[1], (int*)&in_msg[2], (int*)&in_msg[2*in_msg[1] + 2]);
				break;
			case TASK_ALLOCATED:
				map_task_allocated(&mapper, in_msg[1]);
				break;
			case TASK_TERMINATED:
				map_task_terminated(&mapper, in_msg[1]);
				break;
			case TASK_MIGRATION_MAP:
				tm_migrate(&mapper, in_msg[1]);
				break;
			case TASK_MIGRATED:
				tm_migration_complete(&mapper, in_msg[1]);
				break;
			case REQUEST_SERVICE:
				map_request_service(&mapper, in_msg[1], in_msg[2], in_msg[3]);
				break;
			case TASK_ABORTED:
				map_task_aborted(&mapper, in_msg[1]);
				break;
			default:
				printf("Invalid service %x received\n", in_msg[0]);
				break;
		}
	}
}
