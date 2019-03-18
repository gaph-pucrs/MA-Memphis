#include <api.h>
#include <stdlib.h>
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
			in[i][j] = abs(rand(23, 2, 100)%5000);
		}
	}
}


Message msg;

int main(){

	int i, j;
	int pattern[SIZE][SIZE];
	char a[50];

	msg.length = SIZE * SIZE; //SIZE*SIZE nao pode ser maior que 128, senao usar o SendData

	for(j=0; j<PATTERN_PER_TASK; j++){
		for(i=0; i<TOTAL_TASKS; i++){
			randPattern(pattern); //gera uma matriz de valores aleatorios, poderiam ser coeficientes MFCC
			memcpy(msg.msg, pattern, sizeof(pattern));
			msg.length = SIZE * SIZE;
			Send(&msg, P[i]);
			//sprintf(a, "Bank sendedd pattern to task %d", (i+1));
			//Echo(a);
		}
	}

	Echo("Bank sendedd all patterns\n");

	exit();
}
