#include <api.h>
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

int main(int argc, char *argv[])
{
	int m_argc,size,i;
	char *m_argv[3];
	ended = 0;

	//RealTime(103349, 103349, 87835, 0);

	pthread_n_workers = NPROC;

	PROCESSORS = pthread_n_workers;

	execute();

	exit();
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
	int i,j,k;
	k = 0;

	int tasks = NUM_NODES*(NUM_NODES-1)/2;
	int end_task[5] = {0, 0, 0, 0, 0};

	Message msg;
	msg.length = 33;
	while (1) {

		if(end_task[0] != -1)
		{
			Receive(&msg, dijkstra_0);
			for (k=0; k<33; k++)
				result[k] = msg.msg[k];
			//if(result[0] == -1) end_task[0] = -1;
			ProcessMessage();
		}
		if(end_task[1] != -1)
		{
			Receive(&msg, dijkstra_1);
			for (k=0; k<33; k++)
				result[k] = msg.msg[k];
			//if(result[0] == -1) end_task[1] = -1;
			ProcessMessage();
		}
		if(end_task[2] != -1)
		{
			Receive(&msg, dijkstra_2);
			for (k=0; k<33; k++)
				result[k] = msg.msg[k];
			//if(result[0] == -1) end_task[2] = -1;
			ProcessMessage();
		}
		if(end_task[3] != -1)
		{
			Receive(&msg, dijkstra_3);
			for (k=0; k<33; k++)
				result[k] = msg.msg[k];
			//if(result[0] == -1) end_task[3] = -1;
			ProcessMessage();
		}
		if(end_task[4] != -1)
		{
			Receive(&msg, dijkstra_4);
			for (k=0; k<33; k++)
				result[k] = msg.msg[k];
			//if(result[0] == -1) end_task[4] = -1;
			ProcessMessage();
		}

		Echo("########");
		Echo(itoa(GetTick()));

		if (ended == (NPROC)) {
			return 0;
		}
	}

	return 0;
}
