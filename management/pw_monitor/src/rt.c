#include <api.h>
#include <stdlib.h>

#include "rt.h"
#include "monitor.h"

#include "ma_pkg.h"

void rt_test(int id, int waiting_msg, int slack_time, int remaining_time)
{
	// Echo("Waiting = "); Echo(itoa(waiting_msg)); Echo("Slack = "); Echo(itoa(slack_time)); Echo(" Exec = "); Echo(itoa(exec_time)); Echo(" Remaining = "); Echo(itoa(remaining_time));
	if(!waiting_msg && remaining_time > slack_time){
		Echo("Deadline violation detected by task "); Echo(itoa(id)); Echo("\n");

		if(monitor_enabled()){
			Message msg;
			msg.msg[0] = OBSERVE_PACKET;
			msg.msg[1] = id;
			msg.msg[2] = slack_time - remaining_time; /* Quantify deadline miss */
			msg.length = 3;
			SSend(&msg, monitor_get_decider());
		}
	}
}
