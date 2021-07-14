#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>
#include "syn_std.h"

message_t msg;

int main()
{
	
	int i;

	printf("synthetic task A started at %u\n", memphis_get_tick());
	msg.length = 128;
	for(i=0;i<128;i++) msg.payload[i]=i;

for(i=0;i<SYNTHETIC_ITERATIONS;i++){
	memphis_send(&msg,taskB);
	memphis_send(&msg,taskC);
}

    printf("%u\n", memphis_get_tick());
    puts("synthetic task A finished.\n");
	return 0;
}
