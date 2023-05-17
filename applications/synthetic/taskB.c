#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>

#include "syn_std.h"

//MEMPHIS message structure
int msg[128];

int main()
{
    puts("Task B started at time \n");
	//printf("%d\n", memphis_get_tick());

	for(int i=0;i<SYNTHETIC_ITERATIONS;i++)
	{

		memphis_receive(msg, sizeof(msg), taskA);

		compute((unsigned int*)msg);

		memphis_send(msg, sizeof(msg), taskD);
	}

    puts("Task B finished at time\n");
    //printf("%d\n", memphis_get_tick());
	return 0;
}
