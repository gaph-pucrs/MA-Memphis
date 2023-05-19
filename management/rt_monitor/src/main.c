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
 * @brief Main real-time observer file
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <memphis.h>
#include <memphis/monitor.h>
#include <memphis/services.h>

#include "rt.h"

int main()
{
	printf("Real-time monitor started at %d\n", memphis_get_tick());

	oda_t decider;
	oda_init(&decider);
	oda_request_service(&decider, ODA_DECIDE | D_QOS);

	while(true){
		int msg[3];
		memphis_receive_any(msg, sizeof(msg));
		if(msg[0] == SERVICE_PROVIDER && oda_service_provider(&decider, msg[1], msg[2]))
			break;
	}
	// printf("Received service provider %d\n", decider.id);

	size_t slots;
	mon_t *qos_table = mon_create(&slots);

	if(qos_table == NULL){
		puts("FATAL: not enough memory for QOS table");
		exit(errno);
	}

	if(mon_set_dmni(qos_table, MON_QOS) != 0){
		puts("FATAL: Unable to set DMNI table. Exiting.");
		exit(errno);
	}

	mon_announce(MON_QOS);

	while(true){
		memphis_real_time(MON_INTERVAL_QOS, MON_INTERVAL_QOS, 0);	/* Repeat this task every ms */
		for(int i = 0; i < slots; i++){
			if(qos_table[i].task != -1){
				// printf("Task %X has timing of %d\n", qos_table[i].task, qos_table[i].value);
				rt_check(&decider, qos_table[i].task, qos_table[i].value);
				qos_table[i].task = -1;
			}
		}
	}

	return 0;
}
