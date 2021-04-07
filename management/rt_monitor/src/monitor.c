#include <api.h>
#include <stdlib.h>

#include "monitor.h"
#include "ma_pkg.h"
#include "tag.h"

int qos_decide = -1;

void monitor_request_decider()
{
	Message msg;

	/* Ask the mapper whom to send the observe messages */
	msg.msg[0] = REQUEST_SERVICE;
	msg.msg[1] = get_location();
	msg.msg[2] = (DECIDE | D_QOS);
	msg.msg[3] = get_id();
	msg.length = 4;

	SSend(&msg, mapper_task[0]);
}

bool monitor_enabled()
{
	return qos_decide != -1;
}

int monitor_get_decider()
{
	return qos_decide;
}

void monitor_service_provider(int tag, int id)
{
	if(tag == (DECIDE | D_QOS) && id != -1){
		qos_decide = id;
		Echo("Decide task is "); Echo(itoa(qos_decide)); Echo("\n");
	} else {
		/* Invalid answer, request again! */
		Message msg;
		msg.msg[0] = REQUEST_SERVICE;
		msg.msg[1] = get_location();
		msg.msg[2] = (DECIDE | D_QOS);
		msg.msg[3] = get_id();
		msg.length = 4;
		SSend(&msg, mapper_task[0]);
	}
}
