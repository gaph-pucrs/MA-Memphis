#include <stdbool.h>

#include <api.h>
#include <stdlib.h>

#include "ma_pkg.h"

int main()
{
	while(true){
		static Message msg;
		SReceive(&msg);

		switch(msg.msg[0]){
		case MONITOR:
			Echo("Received from LLM: "); Echo(itoa(msg.msg[1])); Echo(itoa(msg.msg[2])); Echo(itoa(msg.msg[3]));
			break;
		}
	}

	return 0;
}
