#include <stdlib.h>
#include <memphis.h>

#define SIZE	11	//tamanho da matriz
#define TOTAL_TASKS	4	//deve ser PAR para dividir igualmente o numero de padroes por task
#define PATTERN_PER_TASK	NUM_PATTERNS/TOTAL_TASKS

#ifndef RT_PERIOD
	#define RT_PERIOD 100000
#endif

#define EXEC_TIME 52000
#define DEADLINE  RT_PERIOD

#define NUM_PATTERNS			80//40


int P[TOTAL_TASKS] = {p1,p2,p3,p4};

int euclideanDistance(int *x, int *y) {
	int ed = 0.0f;
	int aux = 0.0f;
	int i;
	for (i = 0; i < SIZE; i++) {
		aux = x[i] - y[i];
		ed += aux * aux;
	}
	return ed;
}

int min(int x, int y) {
	if (x > y)
		return y;
	return x;
}

int dynamicTimeWarping(int x[SIZE][SIZE], int y[SIZE][SIZE]) {
	int lastCol[SIZE];
	int currCol[SIZE];
	int temp[SIZE];
	int maxI = SIZE - 1;
	int maxJ = SIZE - 1;
	int minGlobalCost;
	int i, j;

	currCol[0] = euclideanDistance(x[0], y[0]);
	for (j = 1; j <= maxJ; j++) {
		currCol[j] = currCol[j - 1] + euclideanDistance(x[0], y[j]);
	}

	for (i = 1; i <= maxI; i++) {

		__builtin_memcpy(temp, lastCol, sizeof(lastCol));
		__builtin_memcpy(lastCol,currCol, sizeof(lastCol));
		__builtin_memcpy(currCol,currCol, sizeof(lastCol));

		currCol[0] = lastCol[0] + euclideanDistance(x[i], y[0]);

		for (j = 1; j <= maxJ; j++) {
			minGlobalCost = min(lastCol[j], min(lastCol[j - 1], currCol[j - 1]));
			currCol[j] = minGlobalCost + euclideanDistance(x[i], y[j]);
		}
	}

	return currCol[maxJ];
}

