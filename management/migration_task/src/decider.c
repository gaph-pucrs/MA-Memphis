#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>

#include "decider.h"
#include "services.h"
#include "tag.h"

int act_migration = -1;

void decider_request_actor()
{
	message_t msg;

	msg.payload[0] = REQUEST_SERVICE;
	msg.payload[1] = memphis_get_addr();
	msg.payload[2] = (ACT | A_MIGRATION);
	msg.payload[3] = memphis_get_id();
	msg.length = 4;

	memphis_send_any(&msg, mapper_task[0]);
}

bool decider_enabled()
{
	return act_migration != -1;
}

int decider_get_actor()
{
	return act_migration;
}

void decider_service_provider(int tag, int id)
{
	if(tag == (ACT | A_MIGRATION) && id != -1){
		act_migration = id;
		printf("Act task is %d\n", act_migration);
	} else {
		/* Invalid answer, request again! */
		message_t msg;

		msg.payload[0] = REQUEST_SERVICE;
		msg.payload[1] = memphis_get_addr();
		msg.payload[2] = (ACT | A_MIGRATION);
		msg.payload[3] = memphis_get_id();
		msg.length = 4;

		memphis_send_any(&msg, mapper_task[0]);
	}
}
