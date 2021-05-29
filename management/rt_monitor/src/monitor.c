#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>

#include "monitor.h"
#include "tag.h"
#include "services.h"

int qos_decide = -1;

void monitor_request_decider()
{
	message_t msg;

	/* Ask the mapper whom to send the observe messages */
	msg.payload[0] = REQUEST_SERVICE;
	msg.payload[1] = memphis_get_addr();
	msg.payload[2] = (DECIDE | D_QOS);
	msg.payload[3] = memphis_get_id();
	msg.length = 4;

	memphis_send_any(&msg, mapper_task[0]);
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
		printf("Decider task is %d\n", qos_decide);
	} else {
		/* Invalid answer, request again! */
		message_t msg;
		msg.payload[0] = REQUEST_SERVICE;
		msg.payload[1] = memphis_get_addr();
		msg.payload[2] = (DECIDE | D_QOS);
		msg.payload[3] = memphis_get_id();
		msg.length = 4;
		memphis_send_any(&msg, mapper_task[0]);
	}
}
