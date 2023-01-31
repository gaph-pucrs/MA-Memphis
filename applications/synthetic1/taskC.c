#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>

#include "../synthetic1/syn_std.h"

int msg[30];

int main()
{
	
	int i, j,t;

	puts("synthetic task C started.\n");
	//printf("%d\n", memphis_get_tick());

for(i=0;i<SYNTHETIC_ITERATIONS;i++){
	
	for(j=0;j<30;j++) msg[j]=i;

	memphis_receive(msg, sizeof(msg), taskA);
	
	for(t=0;t<1000;t++){
	}
	
	memphis_send(msg, sizeof(msg), taskD);
	
	memphis_receive(msg, sizeof(msg), taskB);
	
	for(t=0;t<1000;t++){
	}
	
	memphis_send(msg, sizeof(msg), taskE);

}
    //printf("%d\n", memphis_get_tick());
    puts("synthetic task C finished.\n");

	return 0;
}
