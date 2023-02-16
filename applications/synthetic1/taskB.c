#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>

#include "syn_std.h"

int msg[30];

int main()
{
	
	int i, j,t;

    puts("synthetic task B started.\n");
	//printf("%d\n", memphis_get_tick());
	
for(i=0;i<SYNTHETIC_ITERATIONS;i++){
	for(t=0;t<1000;t++){
	}	
	for(j=0;j<30;j++) msg[j]=i;

	memphis_send(msg, sizeof(msg), taskC);
}

    //printf("%d\n", memphis_get_tick());
    puts("synthetic task B finished.\n");
	return 0;
}
