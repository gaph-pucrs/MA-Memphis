#include <api.h>
#include <stdlib.h>

#include "migration.h"
#include "ma_pkg.h"
#include "tag.h"

void migration_test(int id, int remaining)
{
	if(decider_enabled() && remaining < 0){
		/**
		 * @todo Change to 3 missed deadlines by the same task
		 */
		Message msg;
		msg.msg[0] = TASK_MIGRATION_MAP;
		msg.msg[1] = id;
		msg.length = 2;
		Echo("Requesting migration for task "); Echo(itoa(id)); Echo("\n");

		SSend(&msg, decider_get_actor());
	}
}
