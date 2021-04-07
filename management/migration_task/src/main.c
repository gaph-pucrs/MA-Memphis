#include <stdbool.h>

#include <api.h>
#include <stdlib.h>

#include "decider.h"
#include "migration.h"
#include "ma_pkg.h"
#include "tag.h"

int main()
{
	static Message msg;

	Echo("Migration started at "); Echo(itoa(GetTick())); Echo("\n");
	migration_init();

	decider_request_actor();

	while(true){
		SReceive(&msg);
		switch(msg.msg[0]){
		case OBSERVE_PACKET:
			// Echo("Hello, received observe packet\n");
			migration_test(msg.msg[1], msg.msg[2]);
			break;
		case SERVICE_PROVIDER:
			decider_service_provider(msg.msg[1], msg.msg[2]);
			break;
		}
	}

	return 0;
}
