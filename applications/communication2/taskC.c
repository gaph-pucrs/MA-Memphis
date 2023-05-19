#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>
#include "syn_std.h"

#define MSG_LENGTH 128

int msg[MSG_LENGTH];

int main()
{
	printf("Synthetic task C started at %u\n", memphis_get_tick());
	
	for(int i = 0; i < SYNTHETIC_ITERATIONS; i++){
		memphis_receive(msg, sizeof(msg), taskA);
		for(volatile int i = 0; i < IDLE_CYCLES; i++);
		memphis_send(msg, sizeof(msg) >> 1, taskD);
	}

    printf("Synthetic task C finished at %u\n", memphis_get_tick());
	return 0;
}
