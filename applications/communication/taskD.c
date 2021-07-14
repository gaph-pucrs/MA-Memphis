#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>
#include "syn_std.h"

message_t msg;

int main()
{

	int i;

    printf("synthetic task D started at %u\n", memphis_get_tick());


for(i=0;i<SYNTHETIC_ITERATIONS;i++){
	
	memphis_receive(&msg,taskB);
	memphis_receive(&msg,taskC);

}

	printf("%u\n", memphis_get_tick());
    puts("synthetic task D finished.\n");

	return 0;

}
