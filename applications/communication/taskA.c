#include <api.h>
#include <stdlib.h>
#include "syn_std.h"

Message msg;

int main()
{
	
	int i;

    Echo("synthetic task A started.");
	Echo(itoa(GetTick()));
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
