#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>

#include "../synthetic1/syn_std.h"

message_t msg;

int main()
{
	
	int i, j,t;

	puts("synthetic task C started.\n");
	//printf("%d\n", memphis_get_tick());

for(i=0;i<SYNTHETIC_ITERATIONS;i++){
	
	msg.length = 30;
	for(j=0;j<30;j++) msg.payload[j]=i;

	memphis_receive(&msg,taskA);
	
	for(t=0;t<1000;t++){
	}
	
	memphis_send(&msg,taskD);
	
	memphis_receive(&msg,taskB);
	
	for(t=0;t<1000;t++){
	}
	
	memphis_send(&msg,taskE);

}
    //printf("%d\n", memphis_get_tick());
    puts("synthetic task C finished.\n");

	return 0;
}
