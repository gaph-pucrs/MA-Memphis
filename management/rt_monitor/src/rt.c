/**
 * MA-Memphis
 * @file rt.c
 *
 * @author Angelo Elias Dalzotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date March 2021
 * 
 * @brief Real-time observer functions
 */

#include <memphis.h>
#include <stdio.h>

#include "rt.h"
#include "services.h"

void rt_check(oda_t *decider, int id, int rt_diff)
{
	// Echo("Waiting = "); Echo(itoa(waiting_msg)); Echo("Slack = "); Echo(itoa(slack_time)); Echo(" Exec = "); Echo(itoa(exec_time)); Echo(" Remaining = "); Echo(itoa(remaining_time));
	if(rt_diff < 0){
		printf("Deadline violation detected in task %d\n", id);

		if(oda_is_enabled(decider)){
			message_t msg;
			msg.payload[0] = OBSERVE_PACKET;
			msg.payload[1] = id;
			msg.payload[2] = rt_diff; /* Quantify deadline miss */
			msg.length = 3;
			memphis_send_any(&msg, decider->id);
		}
	}
}
