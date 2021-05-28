#include <stdlib.h>
#include <memphis.h>
#include <stdio.h>

#include "rt.h"
#include "monitor.h"
#include "services.h"

void rt_test(int id, int waiting_msg, int slack_time, int remaining_time)
{
	// Echo("Waiting = "); Echo(itoa(waiting_msg)); Echo("Slack = "); Echo(itoa(slack_time)); Echo(" Exec = "); Echo(itoa(exec_time)); Echo(" Remaining = "); Echo(itoa(remaining_time));
	if(!waiting_msg && remaining_time > slack_time){
		printf("Deadline violation detected in task %d\n", id);

		if(monitor_enabled()){
			message_t msg;
			msg.payload[0] = OBSERVE_PACKET;
			msg.payload[1] = id;
			msg.payload[2] = slack_time - remaining_time; /* Quantify deadline miss */
			msg.length = 3;
			memphis_send_any(&msg, monitor_get_decider());
		}
	}
}
