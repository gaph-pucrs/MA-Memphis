#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>

#include "syn_std.h"

//MEMPHIS message structure
message_t msg1;
message_t msg2;

int main()
{
    puts("Task A started at time \n");
	//printf("%d\n", memphis_get_tick());

	for(int i=0;i<SYNTHETIC_ITERATIONS;i++)
	{
		//Compute and send something
		compute((unsigned int*)&msg1.payload);
		msg1.length = 128;
		memphis_send(&msg1,taskB);

		//Compute and send something
		compute((unsigned int*)&msg2.payload);
		msg2.length = 128;
		memphis_send(&msg2,taskC);
	}

    puts("Task A finished at time\n");
    //printf("%d\n", memphis_get_tick());
	return 0;
}
