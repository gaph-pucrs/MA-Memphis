#include <api.h>
#include <stdlib.h>
#include <stdio.h>

#define NONE                       9999		//Maximum
#define MAXPROCESSORS			   64		//The amount of processor
#define NUM_NODES                  16		//16 for small input; 160 for large input; 30 for medium input;

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

int main(int argc, char *argv[])
{
	int i, j;
	Message msg;
	int rank = 2;

	//RealTime(200000, 200000, 90000, 0);

	qtdEnvios = 0;

	msg.length = NUM_NODES*(NUM_NODES-1)/2;
	Receive(&msg, divider);
	for (i=0; i<(NUM_NODES*(NUM_NODES-1)/2); i++)
		nodes_tasks[i][0] = msg.msg[i];

	Receive(&msg, divider);
	for (i=0; i<(NUM_NODES*(NUM_NODES-1)/2); i++)
		nodes_tasks[i][1] = msg.msg[i];


	msg.length = MAXPROCESSORS;
	Receive(&msg, divider);
	for (i=0; i<MAXPROCESSORS; i++) {
		tasks[i][0] = msg.msg[i];
	}

	Receive(&msg, divider);
	for (i=0; i<MAXPROCESSORS; i++) {
		tasks[i][1] = msg.msg[i];
	}


	msg.length = NUM_NODES;
	for (i=0; i<NUM_NODES; i++) {
		Receive(&msg, divider);
		for (j=0; j<NUM_NODES; j++)
			AdjMatrix[j][i] = msg.msg[j];
	}

	/*for(i=0; i<NUM_NODES; i++) {
		Echo(" D1: ");
		for(j=0; j<NUM_NODES; j++) {
			Echo(itoa(AdjMatrix[i][j]));
			Echo(" ");
		}
		Echo("\n");
	}*/

	dijkstra(rank);

	Echo(itoa(GetTick()));
	Echo("Dijkstra_2 finished.");

	exit();
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

	Message msg;
	msg.length = 33;
	msg.msg[0] = -1;
	for(k=0; k<33; k++)
		msg.msg[k] = resultSend[k];
	Send(&msg, print);
}

void dijkstra(int myID) {
	int x,i,v;
	int chStart, chEnd;
	int u =-1;

	//RealTime(103349, 103349, 11796, 0);

	for(x=tasks[myID][0]; x<tasks[myID][1]; x++){
		chStart = nodes_tasks[x][0];	//Start node
		chEnd = nodes_tasks[x][1];		//End node
		u=-1;

		//Initialize and clear
		uVertex[myID].iDist=NONE;
		uVertex[myID].iPID=myID;
		uVertex[myID].iNID=NONE;
		g_qCount[myID] = 0;
		u=-1;
		for (v=0; v<NUM_NODES; v++) {
			rgnNodes[myID][v].iDist =  AdjMatrix[chStart][v];
			rgnNodes[myID][v].iPrev = NONE;
			rgnNodes[myID][v].iCatched = 0;
		}
		//Start working
		while (qcount(myID) < NUM_NODES-1){
			for (i=0; i<NUM_NODES; i++) {
				if(rgnNodes[myID][i].iCatched==0 && rgnNodes[myID][i].iDist<uVertex[myID].iDist && rgnNodes[myID][i].iDist!=0){
					uVertex[myID].iDist=rgnNodes[myID][i].iDist;
					uVertex[myID].iNID=i;
				}
			}
			globalMiniCost[myID]=NONE;
			if(globalMiniCost[myID]>uVertex[myID].iDist){
				globalMiniCost[myID] = uVertex[myID].iDist;
				u=uVertex[myID].iNID;
				g_qCount[myID]++;
			}
			for (v=0; v<NUM_NODES; v++) {
				if(v==u){
					rgnNodes[myID][v].iCatched = 1;
					continue;
				}
				if((rgnNodes[myID][v].iCatched==0 && rgnNodes[myID][v].iDist>(rgnNodes[myID][u].iDist+AdjMatrix[u][v]))){
					rgnNodes[myID][v].iDist=rgnNodes[myID][u].iDist+AdjMatrix[u][v];
					rgnNodes[myID][v].iPrev = u;
				}
			}
			uVertex[myID].iDist = NONE;	//Reset
		}

		sendResult(myID,chStart,chEnd);
		qtdEnvios++;
	}
	Message msg;
	msg.length = 33;
	msg.msg[0] = -1;
	Send(&msg, print);
	Echo("finaliza\n");
}
