#include <api.h>
#include "dtw.h"

Message msg;

int main(){

	int test[SIZE][SIZE];
	int pattern[SIZE][SIZE];
	int result, j;

	Receive(&msg, recognizer);

	Echo("Task P2 INIT\n");

	memcpy(test, msg.msg, sizeof(test));

	for(j=0; j<PATTERN_PER_TASK; j++){

		memset(msg.msg,0, sizeof(int)*MSG_SIZE);

		Receive(&msg, bank);

		//Echo("Task P2 received pattern from bank\n");

		memcpy(pattern, msg.msg, sizeof(pattern));

		result = dynamicTimeWarping(test, pattern);

		msg.length = 1;

		msg.msg[0] = result;

		Send(&msg, recognizer);
	}

	Echo("Task P2 FINISHEDD IN\n");
	Echo(itoa(GetTick()));

	exit();
}
