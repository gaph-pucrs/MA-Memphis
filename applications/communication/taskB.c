#include <api.h>
#include <stdlib.h>
#include "syn_std.h"

Message msg;

int main()
{
	
	int i, j,t;

    Echo("synthetic task B started.");
	Echo(itoa(GetTick()));
	
for(i=0;i<SYNTHETIC_ITERATIONS;i++){
	Receive(&msg,taskA);
	Send(&msg,taskD);
}

    Echo(itoa(GetTick()));
    Echo("synthetic task B finished.");
	exit();
}
