#include <api.h>
#include <stdlib.h>
#include "syn_std.h"

Message msg;

int main()
{

	int i, j,t;

    Echo("synthetic task D started.");
	Echo(itoa(GetTick()));

for(i=0;i<SYNTHETIC_ITERATIONS;i++){
	
	Receive(&msg,taskB);
	Receive(&msg,taskC);

}

	Echo(itoa(GetTick()));
    Echo("synthetic task D finished.");

	exit();

}
