/**
 * Euler tour application for Memphis
 * 
 * @file manager.c
 * 
 * @author Angelo Elias Dalzotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @brief This file has the source code for the euler tour application manager
 * 
 * @date June 2018
 */

#include <memphis.h>
#include <stdbool.h>
#include <stdio.h>
#include "euler.h"

bool path_only = false;
unsigned int start=NODE_B, end=NODE_C;

/**
 * Verifica se o grafo é euleriano
 * O critério é:
 *  Circuit: todos nodos de grau par
 *  Path: dois nodos ímpares
 */
bool check_eulerian()
{
	int odd_count = 0;
	for(int i = 1; i < TOTAL_TASKS; i++){
		if(deg[i] % 2){   // Sum of odd nodes
			odd_count++;
			if(odd_count == 1){  // Has starting node
				start = i;
				path_only = true;
			} else if(odd_count == 2) // Has ending node
				end = i;
			else                    // Impossible tour
				return false;
		}
	}

	if(odd_count){
		puts("Only path is possible.");
	} else {
		puts("Circuit is possible.");
	}

	return true;
}

int main()
{
	if(!check_eulerian()){
		puts("The graph is not Eulerian. Quiting.");
		return -1;
	}

	message_t msg;
	msg.length = LENGHT;
	MSG_OP = FORWARD;
	MSG_SRC = NODE_M;
	TUNUE = TOTAL_EDGES;
	BK_ADDR = NODE_M;

#if DEBUG == 1
	puts("DEBUG: SEND FORWARD TO START NODE");
#endif
	puts(start_msg[start]);
	memphis_send_any(&msg, P[start]);
#if DEBUG == 1
	puts("DEBUG: SENT FORWARD TO START NODE");
#endif

#if DEBUG == 1
	puts("DEBUG: WAITING EXIT MESSAGE");
#endif
	memphis_receive_any(&msg);
#if DEBUG == 1
	puts("DEBUG: RECEIVED EXIT MESSAGE");
#endif

	puts(exit_msg[MSG_SRC]);
	MSG_OP = EXIT; //Exit
	MSG_SRC = NODE_M; //From manager
	puts("Exiting nodes.");
	for(int i = 1; i < TOTAL_TASKS; i++){
	#if DEBUG == 1
		puts("DEBUG: SEND EXIT TO NODE");
	#endif
		memphis_send_any(&msg, P[i]);
	#if DEBUG == 1
		puts("DEBUG: SENT EXIT TO NODE");
	#endif
	}
	
	puts("Exiting main");
	return 0;
}
