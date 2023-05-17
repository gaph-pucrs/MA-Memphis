#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>

#include "syn_std.h"

//Message structure of MEMPHIS, provided by api.h
int msg[128];

int main()
{
    puts("Task D started at time \n");
	//printf("%d\n", memphis_get_tick());

	for(int i=0;i<SYNTHETIC_ITERATIONS;i++)
	{

		memphis_receive(msg, sizeof(msg), taskB);
		memphis_receive(msg, sizeof(msg), taskC);

	}

	puts("Final message\n");
	for(int j=0; j<128; j++){
		printf("%d\n",msg[j]);
	}


    puts("Task D finished at time\n");
    //printf("%d\n", memphis_get_tick());
	return 0;
}
