#include <api.h>
#include <stdlib.h>

#include "syn_std.h"

//MEMPHIS message structure
Message msg;

void main()
{
    Echo("Task C started at time ");
	Echo(itoa(GetTick()));

	for(int i=0;i<SYNTHETIC_ITERATIONS;i++)
	{

		Receive(&msg, taskA);

		compute(&msg.msg);

		Send(&msg,taskD);
	}

    Echo("Task C finished at time");
    Echo(itoa(GetTick()));
	exit();
}
