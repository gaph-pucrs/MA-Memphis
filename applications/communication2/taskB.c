#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>
#include "syn_std.h"

#define MSG_LENGTH 128

int msg[MSG_LENGTH];

int main()
{
	printf("Synthetic task B started at %u\n", memphis_get_tick());
	
	for(int i = 0; i < SYNTHETIC_ITERATIONS; i++){
		memphis_receive(msg, sizeof(msg), taskA);
		memphis_send(msg, sizeof(msg), taskD);
	}

    printf("Synthetic task B finished at %u\n", memphis_get_tick());
	return 0;
}
