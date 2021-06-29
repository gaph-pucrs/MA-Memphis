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

#include "rt.h"
#include "services.h"
#include "tag.h"

int main()
{
	printf("Real-time monitor started at %d\n", memphis_get_tick());

	oda_t decider;
	oda_init(&decider);
	oda_request_service(&decider, ODA_DECIDE | D_QOS);

	while(true){
		static message_t msg;
		memphis_receive_any(&msg);
		switch(msg.payload[0]){
		case MONITOR:
			// Echo("Received from LLM: "); Echo(itoa(msg.payload[1])); Echo(itoa(msg.payload[2])); Echo(itoa(msg.payload[3])); Echo(itoa(msg.payload[4])); Echo(itoa(msg.payload[5])); Echo("\n");
			rt_check(&decider, msg.payload[1], msg.payload[2], msg.payload[3], msg.payload[4]);
			break;
		case SERVICE_PROVIDER:
			oda_service_provider(&decider, msg.payload[1], msg.payload[2]);
			break;
		}
	}

	return 0;
}
