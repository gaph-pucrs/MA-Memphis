#include <stdbool.h>

#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>

#include "tag.h"

#include "rt.h"
#include "monitor.h"
#include "services.h"

int main()
{
	monitor_request_decider();

	while(true){
		static message_t msg;
		memphis_receive_any(&msg);
		switch(msg.payload[0]){
		case MONITOR:
			// Echo("Received from LLM: "); Echo(itoa(msg.payload[1])); Echo(itoa(msg.payload[2])); Echo(itoa(msg.payload[3])); Echo(itoa(msg.payload[4])); Echo(itoa(msg.payload[5])); Echo("\n");
			rt_test(msg.payload[1], msg.payload[2], msg.payload[3], msg.payload[4]);
			break;
		case SERVICE_PROVIDER:
			monitor_service_provider(msg.payload[1], msg.payload[2]);
			break;
		}
	}

	return 0;
}
