#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <memphis.h>

#include "dtw.h"

/*int pattern[SIZE][SIZE] = {
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
};*/

void randPattern(int in[SIZE][SIZE]){
	int i,j;

	for(i=0; i<SIZE; i++){
		for(j=0; j<SIZE; j++){
			in[i][j] = abs(rand() % 99 + 2);
		}
	}
}

int main()
{
	int pattern[SIZE][SIZE];
	srand(memphis_get_tick());

	for(int j = 0; j < PATTERN_PER_TASK; j++){
		for(int i = 0; i < TOTAL_TASKS; i++){
			randPattern(pattern); //gera uma matriz de valores aleatorios, poderiam ser coeficientes MFCC
			memphis_send(pattern, sizeof(pattern), P[i]);
		}
	}

	puts("Bank sent all patterns");

	return 0;
}
