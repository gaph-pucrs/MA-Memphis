#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>
#include "syn_std.h"

message_t msg;

int main()
{
	
	int i;

	printf("synthetic task B started at %u\n", memphis_get_tick());
	
for(i=0;i<SYNTHETIC_ITERATIONS;i++){
	memphis_receive(&msg,taskA);
	memphis_send(&msg,taskD);
}

    printf("%u\n", memphis_get_tick());
    puts("synthetic task B finished.\n");
	return 0;
}
