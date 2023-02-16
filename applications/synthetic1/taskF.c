#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>

#include "syn_std.h"

int msg[30];

int main()
{

	int i,t;

    puts("synthetic task F started.\n");
	//printf("%d\n", memphis_get_tick());

for(i=0;i<SYNTHETIC_ITERATIONS;i++){
	
		memphis_receive(msg, sizeof(msg), taskE);
		for(t=0;t<1000;t++);
		memphis_receive(msg, sizeof(msg), taskD);

	}

	//printf("%d\n", memphis_get_tick());
    puts("synthetic task F finished.\n");

	return 0;

}
