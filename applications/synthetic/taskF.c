#include <api.h>
#include <stdlib.h>
#include "syn_std.h"

Message msg;

int main()
{

	int i,t;

    Echo("synthetic task F started.");
	Echo(itoa(GetTick()));

for(i=0;i<SYNTHETIC_ITERATIONS;i++){
	
		Receive(&msg,taskE);
			for(t=0;t<1000;t++){
		}
		Receive(&msg,taskD);

	}

	Echo(itoa(GetTick()));
    Echo("synthetic task F finished.");

	exit();

}
