#include <api.h>
#include <stdlib.h>
#include "dtw.h"

Message msg;

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

int main(){

	int i, j;
	//int distances[NUM_PATTERNS];


	msg.length = SIZE*SIZE; //SIZE*SIZE nao pode ser maior que 128, senao usar o SendData

	Echo("Rec ola\n");
	Echo(GetTick());
	memcpy(msg.msg, test, sizeof(test));

	for(i=0; i<TOTAL_TASKS; i++){
		Send(&msg,P[i]);
	}

	Echo("Test Sendedd to all tasks\n");
	//Echo(itoa(PATTERN_PER_TASK));

	for(j=0; j<PATTERN_PER_TASK; j++){
		for(i=0; i<TOTAL_TASKS; i++){
			Receive(&msg, P[i]);
			//distances[d_count] = msg.msg[0];
			//sprintf(d, "DTW entre amostra de teste e o padrão %d = %d  TICK = %d", d_count, distances[d_count], GetTick());
		}
		Echo(itoa(GetTick()));
	}

	/*j = distances[0];

	for(i=1; i<TOTAL_TASKS; i++){
		if(j<distances[i])
			j = distances[i];
	}*/

	Echo("FIM DO RECONHECIMENTO DE PADROES, MENOR DISTANCIA:\n");
	/*Echo(itoa(j));
	Echo("tempo: ");
	Echo(itoa(GetTick()));*/


	exit();
}
