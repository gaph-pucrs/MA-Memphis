#include <api.h>
#include <stdlib.h>

#include "decider.h"
#include "ma_pkg.h"
#include "tag.h"

int act_migration = -1;

void decider_request_actor()
{
	Message msg;

	msg.msg[0] = REQUEST_SERVICE;
	msg.msg[1] = get_location();
	msg.msg[2] = (ACT | A_MIGRATION);
	msg.msg[3] = get_id();
	msg.length = 4;

	SSend(&msg, mapper_task[0]);
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
		Echo("Act task is "); Echo(itoa(act_migration)); Echo("\n");
	} else {
		/* Invalid answer, request again! */
		Message msg;

		msg.msg[0] = REQUEST_SERVICE;
		msg.msg[1] = get_location();
		msg.msg[2] = (ACT | A_MIGRATION);
		msg.msg[3] = get_id();
		msg.length = 4;

		SSend(&msg, mapper_task[0]);
	}
}
