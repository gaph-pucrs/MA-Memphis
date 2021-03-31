#include <stdbool.h>

#include <api.h>
#include <stdlib.h>

#include "ma_pkg.h"
#include "tag.h"

int main()
{
	static Message msg;
	int qos_decide = -1;

	/* Ask the mapper whom to send the observe messages */
	msg.msg[0] = REQUEST_SERVICE;
	msg.msg[1] = get_location();
	msg.msg[2] = (DECIDE | D_QOS);
	msg.msg[3] = get_id();
	msg.length = 4;

	SSend(&msg, mapper_task[0]);

	while(true){
		SReceive(&msg);
		switch(msg.msg[0]){
		case MONITOR:
			Echo("Received from LLM: "); Echo(itoa(msg.msg[1])); Echo(itoa(msg.msg[2])); Echo(itoa(msg.msg[3]));
			if(qos_decide != -1){
				msg.msg[0] = OBSERVE_PACKET;
				msg.msg[1] = msg.msg[1];
				msg.msg[2] = msg.msg[2];
				msg.msg[3] = msg.msg[3];
				msg.length = 4;
				SSend(&msg, qos_decide);
			}
			break;
		case SERVICE_PROVIDER:
			if(msg.msg[1] == (DECIDE | D_QOS) && msg.msg[2] != -1){
				qos_decide = msg.msg[2];
				Echo("Decide task is "); Echo(itoa(qos_decide)); Echo("\n");
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
