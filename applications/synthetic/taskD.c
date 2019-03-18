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
	msg.length = 30;
	for(j=0;j<30;j++) msg.msg[j]=i;
	
		Receive(&msg,taskC);
			for(t=0;t<1000;t++){
		}
		Send(&msg,taskF);

	}

	Echo(itoa(GetTick()));
    Echo("synthetic task D finished.");

	exit();

}
