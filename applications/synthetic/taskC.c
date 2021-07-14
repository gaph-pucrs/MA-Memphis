#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>

#include "syn_std.h"

//MEMPHIS message structure
message_t msg;

int main()
{
    puts("Task C started at time \n");
	//printf("%d\n", memphis_get_tick());

	for(int i=0;i<SYNTHETIC_ITERATIONS;i++)
	{

		memphis_receive(&msg, taskA);

		compute((unsigned int*)&msg.payload);

		memphis_send(&msg,taskD);
	}

    puts("Task C finished at time\nm");
    //printf("%d\n", memphis_get_tick());
	return 0;
}
