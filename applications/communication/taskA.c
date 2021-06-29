#include <api.h>
#include <stdlib.h>
#include "syn_std.h"

Message msg;

int main()
{
	
	int i;

	printf("synthetic task A started at %u\n", memphis_get_tick());
	msg.length = 128;
	for(i=0;i<128;i++) msg.msg[i]=i;

for(i=0;i<SYNTHETIC_ITERATIONS;i++){
	Send(&msg,taskB);
	Send(&msg,taskC);
}

    Echo(itoa(GetTick()));
    Echo("synthetic task A finished.");
	return 0;
}
