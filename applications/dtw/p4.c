#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <memphis.h>

#include "dtw.h"

int main()
{
	static int test[SIZE][SIZE];
	static int pattern[SIZE][SIZE];

	memphis_receive(test, sizeof(test), recognizer);

	printf("Task P%d INIT\n", getpid() & 0xFF);

	for(int j = 0; j < PATTERN_PER_TASK; j++){
		memphis_receive(pattern, sizeof(pattern), bank);

		//puts("Task P1 received pattern from bank\n");

		int result = dynamicTimeWarping(test, pattern);
		printf("R = %d\n", result);

		memphis_send(&result, sizeof(result), recognizer);
	}

	printf("Task FINISHED AT %d\n", memphis_get_tick());

	return 0;
}
