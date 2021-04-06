#include <api.h>
#include <stdlib.h>

#include "rt.h"

void rt_test(int id, int waiting_msg, int slack_time, int exec_time, int remaining_time)
{
	if(!waiting_msg && !slack_time && remaining_time > (exec_time/10)){
		Echo("Deadline violation detected by task "); Echo(itoa(id)); Echo("\n");
	}
}
