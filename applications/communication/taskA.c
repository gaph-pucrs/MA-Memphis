#include <api.h>
#include <stdlib.h>
#include "syn_std.h"

Message msg;

int main()
{
	
	int i, j,t;

    Echo("synthetic task A started.");
	Echo(itoa(GetTick()));
	msg.length = 128;
	for(j=0;j<128;j++) msg.msg[j]=i;

for(i=0;i<SYNTHETIC_ITERATIONS;i++){
	Send(&msg,taskB);
	Send(&msg,taskC);
}

    Echo(itoa(GetTick()));
    Echo("synthetic task A finished.");
	exit();
}
