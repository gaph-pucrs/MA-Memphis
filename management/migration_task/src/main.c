#include <api.h>
#include <stdlib.h>

#include "pkg.h"

#warning Migration depends on mapper. Please include it in the testcase.

int main()
{
	Echo("Migration started at "); Echo(itoa(GetTick())); Echo("\n");

	/**
	 * @todo
	 * Receive monitoring information and decide when to trigger migration
	 */

	/* Action: trigger migration for task 257. Mapper will decide where to migrate the task */
	Message msg;
	msg.msg[0] = TASK_MIGRATION_MAP;
	msg.msg[1] = 516;
	msg.length = 2;

	while(GetTick() < 800000);

	Echo("Requesting migration for task "); Echo(itoa(msg.msg[1])); Echo("\n");

	SSend(&msg, mapper_task[0]);

	return 0;
}
