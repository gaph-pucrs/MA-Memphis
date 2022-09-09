#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>

#define NUM_NODES                  16		//16 for small input; 160 for large input; 30 for medium input;
#define MAXPROCESSORS			   64		//The amount of processor
#define NPROC 						5

#define MSG_LEN 	(NUM_NODES*(NUM_NODES-1)/2)
#define MSG_SIZE	(MSG_LEN << 2)

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


void startThreads(void) {
	static int msg[MSG_LEN];

	/* SEND nodes_tasks[NUM_NODES*(NUM_NODES-1)/2][2] */
	// Send X of nodes_tasks
	for(int i = 0; i < MSG_LEN; i++){
		msg[i] = nodes_tasks[i][0];
	}

	memphis_send(msg, MSG_SIZE, dijkstra_0);
	puts("Sent X nodes_tasks");
	memphis_send(msg, MSG_SIZE, dijkstra_1);
	puts("Sent X nodes_tasks");
	memphis_send(msg, MSG_SIZE, dijkstra_2);
	puts("Sent X nodes_tasks");
	memphis_send(msg, MSG_SIZE, dijkstra_3);
	puts("Sent X nodes_tasks");
	memphis_send(msg, MSG_SIZE, dijkstra_4);
	puts("Sent X nodes_tasks");

	// Send Y of nodes_tasks
	for(int i = 0; i < MSG_LEN; i++)
		msg[i] = nodes_tasks[i][1];

	memphis_send(msg, MSG_SIZE, dijkstra_0);
	puts("Sent Y nodes_tasks");
	memphis_send(msg, MSG_SIZE, dijkstra_1);
	puts("Sent Y nodes_tasks");
	memphis_send(msg, MSG_SIZE, dijkstra_2);
	puts("Sent Y nodes_tasks");
	memphis_send(msg, MSG_SIZE, dijkstra_3);
	puts("Sent Y nodes_tasks");
	memphis_send(msg, MSG_SIZE, dijkstra_4);
	puts("Sent Y nodes_tasks");

	/* SEND tasks[MAXPROCESSORS][2] */
	// Send X of tasks
	for(int i = 0; i < MAXPROCESSORS; i++)
		msg[i] = tasks[i][0];

	memphis_send(msg, MAXPROCESSORS << 2, dijkstra_0);
	puts("Sent X tasks");
	memphis_send(msg, MAXPROCESSORS << 2, dijkstra_1);
	puts("Sent X tasks");
	memphis_send(msg, MAXPROCESSORS << 2, dijkstra_2);
	puts("Sent X tasks");
	memphis_send(msg, MAXPROCESSORS << 2, dijkstra_3);
	puts("Sent X tasks");
	memphis_send(msg, MAXPROCESSORS << 2, dijkstra_4);
	puts("Sent X tasks");

	// Send Y of tasks
	for(int i = 0; i < MAXPROCESSORS; i++)
		msg[i] = tasks[i][1];

	memphis_send(msg, MAXPROCESSORS << 2, dijkstra_0);
	puts("Sent Y tasks");
	memphis_send(msg, MAXPROCESSORS << 2, dijkstra_1);
	puts("Sent Y tasks");
	memphis_send(msg, MAXPROCESSORS << 2, dijkstra_2);
	puts("Sent Y tasks");
	memphis_send(msg, MAXPROCESSORS << 2, dijkstra_3);
	puts("Sent Y tasks");
	memphis_send(msg, MAXPROCESSORS << 2, dijkstra_4);
	puts("Sent Y tasks");

	/* SEND AdjMatrix[NUM_NODES][NUM_NODES] */
	for(int i = 0; i < NUM_NODES; i++){
		for(int j = 0; j < NUM_NODES; j++){
			msg[j] = AdjMatrix[j][i];
		}
		memphis_send(msg, NUM_NODES << 2, dijkstra_0);
		memphis_send(msg, NUM_NODES << 2, dijkstra_1);
		memphis_send(msg, NUM_NODES << 2, dijkstra_2);
		memphis_send(msg, NUM_NODES << 2, dijkstra_3);
		memphis_send(msg, NUM_NODES << 2, dijkstra_4);
	}

	puts("Sent AdjMatrix");
}

void divide_task_group(int task) {
	int i;
	for(i=0;i<PROCESSORS;i++){
		tasks[i][0] = task/PROCESSORS* (i);
		tasks[i][1] = task/PROCESSORS* (i+1) + (i+1 == PROCESSORS && task % PROCESSORS !=0 ? task % PROCESSORS : 0);

	}
}

int execute() {

	int i,j,k;
	k = 0;

	int fpTrix[NUM_NODES*NUM_NODES] = { 1,    6,    3,    9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999,
										6,    1,    2,    5,    9999, 9999, 1,    9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999,
										3,    2,    1,    3,    4,    9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999,
										9999, 5,    3,    1,    2,    3,    9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999,
										9999, 9999, 4,    2,    1,    5,    9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999,
										9999, 9999, 9999, 3,    5,    1,    3,    2,    9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999,
										9999, 1,    9999, 9999, 9999, 3,    1,    4,    9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999,
										9999, 9999, 9999, 9999, 9999, 2,    4,    1,    7,    9999, 9999, 9999, 9999, 9999, 9999, 9999,
										9999, 9999, 9999, 9999, 9999, 9999, 9999, 7,    1,    5,    1,    9999, 9999, 9999, 9999, 9999,
										9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 5,    1,    9999, 3,    9999, 9999, 9999, 9999,
										9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 1,    9999, 1,    9999, 4,    9999, 9999, 8,
										9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 3,    9999, 1,    9999, 2,    9999, 9999,
										9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 4,    9999, 1,    1,    9999, 2,
										9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 2,    1,    1,    6,    9999,
										9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 6,    1,    3,
										9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 8,    9999, 2,    9999, 3,    1 };

	/* Step 1: geting the working vertexs and assigning values */
	for (i=0;i<NUM_NODES;i++) {
		for (j=0;j<NUM_NODES;j++) {
			AdjMatrix[i][j]= fpTrix[k];
			k++;
		}
	}

	int tasks = NUM_NODES*(NUM_NODES-1)/2;

	int x=0;
	for(i=0;i<NUM_NODES-1;i++){ //small:15; large:159
		for(j=i+1;j<NUM_NODES;j++){	//small:16; large:160
			nodes_tasks[x][0] = i;
			nodes_tasks[x][1] = j;
			x++;
		}
	}

	divide_task_group(tasks);
	startThreads();

	return 0;
}
