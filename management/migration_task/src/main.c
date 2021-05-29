#include <stdbool.h>

#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>

#include "decider.h"
#include "migration.h"
#include "tag.h"
#include "services.h"

int main()
{
	static message_t msg;

	printf("Migration started at %d\n", memphis_get_tick());
	
	migration_init();

	decider_request_actor();

	while(true){
		memphis_receive_any(&msg);
		switch(msg.payload[0]){
		case OBSERVE_PACKET:
			// Echo("Hello, received observe packet\n");
			migration_test(msg.payload[1], msg.payload[2]);
			break;
		case SERVICE_PROVIDER:
			decider_service_provider(msg.payload[1], msg.payload[2]);
			break;
		}
	}

	return 0;
}
