#include <api.h>
#include <stdlib.h>

#include "syn_std.h"

//Message structure of MEMPHIS, provided by api.h
Message msg;

int main()
{
    Echo("Task D started at time ");
	Echo(itoa(GetTick()));

	for(int i=0;i<SYNTHETIC_ITERATIONS;i++)
	{

		Receive(&msg, taskB);
		Receive(&msg, taskC);

	}

	Echo("Final message");
	for(int j=0; j<msg.length; j++){
		Echo(itoa(msg.msg[j]));
	}


    Echo("Task D finished at time");
    Echo(itoa(GetTick()));
	return 0;
}
