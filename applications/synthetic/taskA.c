#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>

#include "syn_std.h"

//MEMPHIS message structure
int msg1[128];

int main()
{
    puts("Task A started at time \n");
	//printf("%d\n", memphis_get_tick());

	for(int i=0;i<SYNTHETIC_ITERATIONS;i++)
	{
		//Compute and send something
		compute((unsigned int*)msg1);
		memphis_send(msg1, sizeof(msg1), taskB);

		//Compute and send something
		compute((unsigned int*)msg1);
		memphis_send(msg1, sizeof(msg1), taskC);
	}

    puts("Task A finished at time\n");
    //printf("%d\n", memphis_get_tick());
	return 0;
}
