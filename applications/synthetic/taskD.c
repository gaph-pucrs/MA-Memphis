#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>

#include "syn_std.h"

//Message structure of MEMPHIS, provided by api.h
message_t msg;

int main()
{
    puts("Task D started at time \n");
	//printf("%d\n", memphis_get_tick());

	for(int i=0;i<SYNTHETIC_ITERATIONS;i++)
	{

		memphis_receive(&msg, taskB);
		memphis_receive(&msg, taskC);

	}

	puts("Final message\n");
	for(int j=0; j<msg.length; j++){
		printf("%d\n",msg.payload[j]);
	}


    puts("Task D finished at time\n");
    //printf("%d\n", memphis_get_tick());
	return 0;
}
