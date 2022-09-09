#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>

#define NUM_NODES                  16		//16 for small input; 160 for large input; 30 for medium input;
#define MAXPROCESSORS			   64		//The amount of processor
#define NPROC 						5

int PROCESSORS;
int pthread_n_workers;
int paths;
int tasks[MAXPROCESSORS][2];
int nodes_tasks[NUM_NODES*(NUM_NODES-1)/2][2];
int AdjMatrix[NUM_NODES][NUM_NODES];
int result[33];
int ended;

int execute();

int main(int argc, char *argv[])
{
	// int m_argc,size,i;
	// char *m_argv[3];
	ended = 0;

	pthread_n_workers = NPROC;

	PROCESSORS = pthread_n_workers;

	execute();

	return 0;
}

void ProcessMessage() {
	volatile int paths = 0;

	if (result[0] == -1) {
		ended++;
	}
	else {
		for(paths=3; paths<33; paths++);
	}
}

int execute() {
	int k;
	k = 0;

	// int tasks = NUM_NODES*(NUM_NODES-1)/2;
	int end_task[5] = {0, 0, 0, 0, 0};

	static int msg[33];
	while (1) {

		if(end_task[0] != -1)
		{
			memphis_receive(msg, 33 << 2, dijkstra_0);
			for (k=0; k<33; k++)
				result[k] = msg[k];
			//if(result[0] == -1) end_task[0] = -1;
			ProcessMessage();
		}
		if(end_task[1] != -1)
		{
			memphis_receive(msg, 33 << 2, dijkstra_1);
			for (k=0; k<33; k++)
				result[k] = msg[k];
			//if(result[0] == -1) end_task[1] = -1;
			ProcessMessage();
		}
		if(end_task[2] != -1)
		{
			memphis_receive(msg, 33 << 2, dijkstra_2);
			for (k=0; k<33; k++)
				result[k] = msg[k];
			//if(result[0] == -1) end_task[2] = -1;
			ProcessMessage();
		}
		if(end_task[3] != -1)
		{
			memphis_receive(msg, 33 << 2, dijkstra_3);
			for (k=0; k<33; k++)
				result[k] = msg[k];
			//if(result[0] == -1) end_task[3] = -1;
			ProcessMessage();
		}
		if(end_task[4] != -1)
		{
			memphis_receive(msg, 33 << 2, dijkstra_4);
			for (k=0; k<33; k++)
				result[k] = msg[k];
			//if(result[0] == -1) end_task[4] = -1;
			ProcessMessage();
		}

		puts("########\n");
		//printf("%d\n", memphis_get_tick());

		if (ended == (NPROC)) {
			return 0;
		}
	}

	return 0;
}
