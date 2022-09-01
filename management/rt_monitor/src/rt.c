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

#include "rt.h"

#include <stdio.h>

#include <memphis.h>
#include <memphis/services.h>

void rt_check(oda_t *decider, int id, int rt_diff)
{
	// Echo("Waiting = "); Echo(itoa(waiting_msg)); Echo("Slack = "); Echo(itoa(slack_time)); Echo(" Exec = "); Echo(itoa(exec_time)); Echo(" Remaining = "); Echo(itoa(remaining_time));
	if(rt_diff < 0){
		printf("Deadline violation detected in task %d\n", id);

		if(oda_is_enabled(decider)){
			int msg[] = {
				OBSERVE_PACKET,
				id,
				rt_diff
			};
			memphis_send_any(msg, sizeof(msg), oda_get_id(decider));
		}
	}
}
