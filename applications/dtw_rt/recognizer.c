#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "dtw.h"

/*int test[SIZE][SIZE] = {
		{7200, 4600},
		{1900, 5800}
};*/

int test[SIZE][SIZE] = {
	{0,1,2,3,4,5,6,7,8,9,0},
	{0,1,2,3,4,5,6,7,8,9,0},
	{0,1,2,3,4,5,6,7,8,9,0},
	{0,1,2,3,4,5,6,7,8,9,0},
	{0,1,2,3,4,5,6,7,8,9,0},
	{0,1,2,3,4,5,6,7,8,9,0},
	{0,1,2,3,4,5,6,7,8,9,0},
	{0,1,2,3,4,5,6,7,8,9,0},
	{0,1,2,3,4,5,6,7,8,9,0},
	{0,1,2,3,4,5,6,7,8,9,0},
	{0,1,2,3,4,5,6,7,8,9,0}
};

int main()
{
	static int results[PATTERN_PER_TASK][TOTAL_TASKS];
	puts("Recognizer start");
	
	for(int i = 0; i < TOTAL_TASKS; i++)
		memphis_send(test, sizeof(test), P[i]);

	puts("Test sent to all tasks");

	for(int j = 0; j < PATTERN_PER_TASK; j++){
		for(int i = 0; i < TOTAL_TASKS; i++){
			memphis_receive(&results[j][i], sizeof(int), P[i]);
		}
	}

	int smaller = results[0][0];
	for(int j = 0; j < PATTERN_PER_TASK; j++){
		for(int i = 0; i < TOTAL_TASKS; i++){
			if(results[j][i] < smaller)
				smaller = results[j][i];
		}
	}

	printf("Pattern recognition finished. Smallest value: %d\n", smaller);

	return 0;
}
