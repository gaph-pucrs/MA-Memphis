#include <stdbool.h>

#include <api.h>
#include <stdlib.h>

#include "mapper.h"
#include "task_migration.h"

int main()
{
	Echo("Mapper started at "); Echo(itoa(GetTick())); Echo("\n");

	mapper_t mapper;
	map_init(&mapper);

	while(true){
		static Message msg;
		SReceive(&msg);
		/* Check what service has been received */
		switch(msg.msg[0]){
			case NEW_APP:
				map_new_app(&mapper, msg.msg[2], &msg.msg[3], msg.msg[1] - 1);
				break;
			case TASK_ALLOCATED:
				map_task_allocated(&mapper, msg.msg[1]);
				break;
			case TASK_TERMINATED:
				map_task_terminated(&mapper, msg.msg[1]);
				break;
			case TASK_MIGRATION_MAP:
				tm_migrate(&mapper, msg.msg[1]);
				break;
			case TASK_MIGRATED:
				tm_migration_complete(&mapper, msg.msg[1]);
				break;
			case REQUEST_SERVICE:
				map_request_service(&mapper, msg.msg[1], msg.msg[2], msg.msg[3]);
				break;
			default:
				Echo("Invalid service received: "); Echo(itoa(msg.msg[0])); Echo("\n");
				break;
		}
	}
}
