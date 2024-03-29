#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <memphis.h>

#include "dijkstra.h"

#define NONE                       9999		//Maximum
#define MAXPROCESSORS			   64		//The amount of processor
#define NUM_NODES                  16		//16 for small input; 160 for large input; 30 for medium input;

#define MSG_LEN 	NUM_NODES*(NUM_NODES-1)/2
#define MSG_SIZE	MSG_LEN << 2

struct _NODE{
	int iDist;
	int iPrev;
	int iCatched;
};
typedef struct _NODE NODE;

struct _UVERTEX{
	int iPID;
	int iNID;
	int iDist;
};
typedef struct _UVERTEX UVERTEX;

UVERTEX uVertex[MAXPROCESSORS];
NODE rgnNodes[MAXPROCESSORS][NUM_NODES];
int g_qCount[MAXPROCESSORS];
int paths;
int resultSend[33];

int tasks[MAXPROCESSORS][2];
int nodes_tasks[NUM_NODES*(NUM_NODES-1)/2][2];
int AdjMatrix[NUM_NODES][NUM_NODES];

int globalMiniCost[MAXPROCESSORS];
int qtdEnvios;

int qcount (int myID);
void sendResult(int myID,int chStart, int chEnd);

int main(int argc, char *argv[])
{
	int msg[MSG_LEN];
	int rank = 0;

	qtdEnvios = 0;

	memphis_receive(msg, MSG_SIZE, divider);

	for(int i = 0; i < (NUM_NODES*(NUM_NODES-1)/2); i++)
		nodes_tasks[i][0] = msg[i];

	memphis_receive(msg, MSG_SIZE, divider);

	for(int i = 0; i < (NUM_NODES*(NUM_NODES-1)/2); i++)
		nodes_tasks[i][1] = msg[i];

	memphis_receive(msg, MAXPROCESSORS << 2, divider);

	/*Echo("\n OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO msg.msg[i]");
	Echo(itoa(msg.msg[0]));
	Echo("\n");*/
	for(int i = 0; i < MAXPROCESSORS; i++){
		tasks[i][0] = msg[i];
		/*Echo("\n pppppppppppppppppppppp tasks[i][0]");
		Echo(itoa(tasks[i][0]));
		Echo("\n");*/
	}

	memphis_receive(msg, MAXPROCESSORS << 2, divider);

	/*Echo("\n OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO msg.msg[i]");
	Echo(itoa(msg.msg[0]));
	Echo("\n");*/
	for(int i = 0; i < MAXPROCESSORS; i++){
		tasks[i][1] = msg[i];
		/*Echo("\n pppppppppppppppppppppp tasks[i][1]");
		Echo(itoa(tasks[i][1]));
		Echo("\n");*/
	}

	for(int i = 0; i < NUM_NODES; i++){
		memphis_receive(msg, NUM_NODES << 2, divider);
		for(int j = 0; j < NUM_NODES; j++)
			AdjMatrix[j][i] = msg[j];
	}

	/*for(i=0; i<NUM_NODES; i++) {
		Echo(" D1: ");
		for(j=0; j<NUM_NODES; j++) {
			Echo(itoa(AdjMatrix[i][j]));
			Echo(" ");
		}
		Echo("\n");
	}*/

	//dijkstra(rank);

	int chStart, chEnd;
	int u =-1;

	memphis_real_time(DEADLINE, DEADLINE, EXEC_TIME);

	for(int x = tasks[rank][0]; x < tasks[rank][1]; x++){
		chStart = nodes_tasks[x][0];	//Start node
		chEnd = nodes_tasks[x][1];		//End node
		u=-1;

		//Initialize and clear
		uVertex[rank].iDist=NONE;
		uVertex[rank].iPID=rank;
		uVertex[rank].iNID=NONE;
		g_qCount[rank] = 0;
		u=-1;
		for(int v = 0; v < NUM_NODES; v++){
			rgnNodes[rank][v].iDist =  AdjMatrix[chStart][v];
			rgnNodes[rank][v].iPrev = NONE;
			rgnNodes[rank][v].iCatched = 0;
		}
		//Start working
		while (qcount(rank) < NUM_NODES-1){
			for(int i = 0; i < NUM_NODES; i++){
				if(rgnNodes[rank][i].iCatched==0 && rgnNodes[rank][i].iDist<uVertex[rank].iDist && rgnNodes[rank][i].iDist!=0){
					uVertex[rank].iDist=rgnNodes[rank][i].iDist;
					uVertex[rank].iNID=i;
				}
			}
			globalMiniCost[rank]=NONE;
			if(globalMiniCost[rank]>uVertex[rank].iDist){
				globalMiniCost[rank] = uVertex[rank].iDist;
				u=uVertex[rank].iNID;
				g_qCount[rank]++;
			}
			for(int v = 0; v < NUM_NODES; v++){
				if(v==u){
					rgnNodes[rank][v].iCatched = 1;
					continue;
				}
				if((rgnNodes[rank][v].iCatched==0 && rgnNodes[rank][v].iDist>(rgnNodes[rank][u].iDist+AdjMatrix[u][v]))){
					rgnNodes[rank][v].iDist=rgnNodes[rank][u].iDist+AdjMatrix[u][v];
					rgnNodes[rank][v].iPrev = u;
				}
			}
			uVertex[rank].iDist = NONE;	//Reset
		}

		sendResult(rank,chStart,chEnd);
		qtdEnvios++;
	}

	//Message msg;
	msg[0] = -1;
	memphis_send(msg, 33 << 2, print);

	//printf("%d", memphis_get_teck());
	printf("Dijkstra_%d finished.\n", getpid());



	/*Echo("\n OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO tasks[rank][0]");
	Echo(itoa(tasks[rank][0]));
	Echo("\n");

	Echo("\n OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO tasks[rank][1]");
	Echo(itoa(tasks[rank][1]));
	Echo("\n");*/

	return 0;
}

int qcount (int myID){
	return(g_qCount[myID]);
}

void sendPath(NODE *rgnNodes, int chNode){
	if ((rgnNodes+chNode)->iPrev != NONE){
		sendPath(rgnNodes, (rgnNodes+chNode)->iPrev);
	}
	resultSend[paths] = chNode+1;
	paths++;
}

void sendResult(int myID,int chStart, int chEnd){
	paths = 3;
	int k;
	for(k=0; k<33; k++)
		resultSend[k] = 0;
	resultSend[0] = chStart;
	resultSend[1] = chEnd;
	resultSend[2] = rgnNodes[myID][chEnd].iDist;
	sendPath(rgnNodes[myID], chEnd);

	int msg[33];

	for(k=0; k<33; k++)
		msg[k] = resultSend[k];
	memphis_send(msg, 33 << 2, print);
}

void dijkstra(int myID) {

}
