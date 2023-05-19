#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>
#include "syn_std.h"

#define MSG_LENGTH 128

int msg[MSG_LENGTH];

int main()
{
	printf("Synthetic task A started at %u\n", memphis_get_tick());
	for(int i = 0; i < MSG_LENGTH; i++)
		msg[i] = i;

	for(int i = 0; i < SYNTHETIC_ITERATIONS; i++){
		memphis_send(msg, sizeof(msg), taskB);
		memphis_send(msg, sizeof(msg) >> 1, taskC);
	}

    printf("Synthetic task A finished at %u\n", memphis_get_tick());

	return 0;
}
