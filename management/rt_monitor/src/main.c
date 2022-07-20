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

#include <memphis.h>
#include <monitor.h>

#include "rt.h"
#include "services.h"

MONITOR_TABLE(qos_table);

int main()
{
	printf("Real-time monitor started at %d\n", memphis_get_tick());

	oda_t decider;
	oda_init(&decider);
	oda_request_service(&decider, ODA_DECIDE | D_QOS);

	while(true){
		message_t msg;
		memphis_receive_any(&msg);
		if(msg.payload[0] == SERVICE_PROVIDER && oda_service_provider(&decider, msg.payload[1], msg.payload[2]))
			break;
	}
	// printf("Received service provider %d\n", decider.id);

	monitor_init(qos_table);

	if(monitor_set_dmni(qos_table, MON_QOS) != 0){
		puts("ERROR: Unable to set DMNI table. Exiting.");
		exit(errno);
	}

	monitor_announce(MON_QOS);

	while(true){
		memphis_real_time(PKG_MONITOR_INTERVAL_QOS, PKG_MONITOR_INTERVAL_QOS, 0);	/* Repeat this task every ms */
		for(int n = 0; n < PKG_N_PE; n++){
			for(int t = 0; t < PKG_MAX_LOCAL_TASKS; t++){
				if(qos_table[n][t].task != -1){
					// printf("Task %X has timing of %X\n", qos_table[n][t].task, (int)qos_table[n][t].value);
					rt_check(&decider, qos_table[n][t].task, qos_table[n][t].value);
					qos_table[n][t].task = -1;
				}
			}
		}
	}

	return 0;
}
