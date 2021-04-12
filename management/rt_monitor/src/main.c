#include <stdbool.h>

#include <api.h>
#include <stdlib.h>

#include "ma_pkg.h"
#include "tag.h"

#include "rt.h"
#include "monitor.h"

int main()
{
	monitor_request_decider();

	while(true){
		static Message msg;
		SReceive(&msg);
		switch(msg.msg[0]){
		case MONITOR:
			// Echo("Received from LLM: "); Echo(itoa(msg.msg[1])); Echo(itoa(msg.msg[2])); Echo(itoa(msg.msg[3])); Echo(itoa(msg.msg[4])); Echo(itoa(msg.msg[5])); Echo("\n");
			rt_test(msg.msg[1], msg.msg[2], msg.msg[3], msg.msg[4]);
			break;
		case SERVICE_PROVIDER:
			monitor_service_provider(msg.msg[1], msg.msg[2]);
			break;
		}
	}

	return 0;
}
