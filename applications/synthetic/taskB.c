#include <api.h>
#include <stdlib.h>

#include "syn_std.h"

//MEMPHIS message structure
Message msg;

int main()
{
    Echo("Task B started at time ");
	Echo(itoa(GetTick()));

	for(int i=0;i<SYNTHETIC_ITERATIONS;i++)
	{

		Receive(&msg, taskA);

		compute((unsigned int*)&msg.msg);

		Send(&msg,taskD);
	}

    Echo("Task B finished at time");
    Echo(itoa(GetTick()));
	return 0;
}
