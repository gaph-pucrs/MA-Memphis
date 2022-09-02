/**
 * MA-Memphis
 * @file main.c
 *
 * @author Angelo Elias Dal Zotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date March 2021
 * 
 * @brief Main file of the MA-Memphis mapper
 */

#include <stdio.h>

#include <memphis.h>
#include <memphis/services.h>

#include "mapper.h"

#define MSG_FLITS 128

int main()
{
	static const size_t MSG_BYTES = MSG_FLITS << 2;
	printf("Mapper task started at time %u\n", memphis_get_tick());

	static map_t mapper;
	map_init(&mapper);

	while(true){
		static int message[MSG_FLITS];
		memphis_receive_any(message, MSG_BYTES);

		switch(message[0]){
			case NEW_APP:
				map_new_app(&mapper, message[1], message[2], &message[3], &message[(message[2] << 1) + 3]);
				break;
			case TASK_ALLOCATED:
				map_task_allocated(&mapper, message[1]);
				break;
			case TASK_TERMINATED:
				map_task_terminated(&mapper, message[1]);
				break;
			case TASK_ABORTED:
				map_task_aborted(&mapper, message[1]);
				break;
			case REQUEST_SERVICE:
				map_request_service(&mapper, message[1], message[2], message[3]);
				break;
			case TASK_MIGRATION_MAP:
				map_migration_map(&mapper, message[1]);
				break;
			default:
				printf("Invalid service %x\n", message[0]);
				break;
		}
	}

	return 0;
}
