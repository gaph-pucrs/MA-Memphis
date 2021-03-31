#include <stdbool.h>

#include <api.h>
#include <stdlib.h>

#include "ma_pkg.h"
#include "tag.h"

#warning Migration depends on mapper. Please include it in the testcase.

int main()
{
	static Message msg;
	int act_migration = -1;

	Echo("Migration started at "); Echo(itoa(GetTick())); Echo("\n");

	msg.msg[0] = REQUEST_SERVICE;
	msg.msg[1] = get_location();
	msg.msg[2] = (ACT | A_MIGRATION);
	msg.msg[3] = get_id();
	msg.length = 4;
	SSend(&msg, mapper_task[0]);

	while(true){
		SReceive(&msg);
		switch(msg.msg[0]){
		case OBSERVE_PACKET:
			Echo("Hello, received observe packet\n");
			if(act_migration != -1 && false){
				msg.msg[0] = TASK_MIGRATION_MAP;
				msg.msg[1] = 516;
				msg.length = 2;
				Echo("Requesting migration for task "); Echo(itoa(msg.msg[1])); Echo("\n");

				SSend(&msg, mapper_task[0]);
			}
			break;
		case SERVICE_PROVIDER:
			if(msg.msg[1] == (ACT | A_MIGRATION) && msg.msg[2] != -1){
				act_migration = msg.msg[2];
				Echo("Act task is "); Echo(itoa(act_migration)); Echo("\n");
			} else {
				/* Invalid answer, request again! */
				msg.msg[0] = REQUEST_SERVICE;
				msg.msg[1] = get_location();
				msg.msg[2] = (DECIDE | D_QOS);
				msg.msg[3] = get_id();
				msg.length = 4;
				SSend(&msg, mapper_task[0]);
			}
			break;
		}
	}

	return 0;
}
