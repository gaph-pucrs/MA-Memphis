/**
 * Euler tour application for Memphis
 * 
 * @file node_b.c
 * 
 * @author Angelo Elias Dalzotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @brief This file has the source code for a node and must be replicated for 
 * each new node added to the application.
 * 
 * @date June 2018
 */

#include <api.h>
#include <stdbool.h>
#include "euler.h"


/* Node Model */
#define THIS_NODE NODE_B
// Number of neighbors
#define N_ADJ 2
// Edges weight           to: a  b
unsigned int weight[N_ADJ] = {0, 0};


//DONT CHANGE
unsigned int counterf = 0;
unsigned int node_i = 0;
unsigned int backtracks = 0;

Message msg;

void next_hop()
{
	MSG_SRC = THIS_NODE;
	if(N_ADJ - counterf - backtracks > 1)
		BK_ADDR = THIS_NODE;

	if(N_ADJ - counterf - backtracks){ // Send Forward
		MSG_OP = FORWARD;
		TUNUE--;
		for(int i = 0; i < N_ADJ; i++){
			if(!weight[i]){
				weight[i] = ++counterf;
			#if DEBUG == 1
				Echo("DEBUG: SEND FORWARD");
			#endif
				Echo(fwd_msg[atop[THIS_NODE][i]]);
				SSend(&msg, P[atop[THIS_NODE][i]]);
			#if DEBUG == 1
				Echo("DEBUG: SENT FORWARD");
			#endif
				break;
			}
		}
	} else { // Send Backtrack
		MSG_OP = BACKTRACK;
	#if DEBUG == 1
		Echo("DEBUG: SEND BACKTRACK");
	#endif
		Echo(bkt_msg[BK_ADDR]);
		SSend(&msg, P[BK_ADDR]);
	#if DEBUG == 1
		Echo("DEBUG: SENT BACKTRACK");
	#endif
	}
}


int main()
{
	Echo(start_msg[THIS_NODE]);
	while(true){
	#if DEBUG == 1
		Echo("DEBUG: WAITING COMMAND");
	#endif
		SReceive(&msg);
	#if DEBUG == 1
		Echo("DEBUG: COMMAND RECEIVED");
	#endif

		if(!MSG_OP){        // Exit
			Echo(exit_msg[THIS_NODE]);
			return 0;
		} else if(!TUNUE){
			MSG_OP = EXIT;
		#if DEBUG == 1
			Echo("DEBUG: SEND EXIT REQUEST TO MANAGER");
		#endif
			Echo("No more unused edges. Exiting.");
			SSend(&msg, P[0]);
		#if DEBUG == 1
			Echo("DEBUG: SENT EXIT REQUEST TO MANAGER");
		#endif
		} else if(MSG_OP == FORWARD){
			Echo(rcv_msg[MSG_SRC]);
			if(MSG_SRC)
				weight[ntoa[THIS_NODE][MSG_SRC]] = ++counterf;
			
			next_hop();
		} else { // Backtrack
			// Find backtracked edge
			Echo(bkt_msg[MSG_SRC]);
			int top = 0;
			for(int i = 0; i < N_ADJ; i++){
				if(weight[i]>weight[top])
					top = i;
			}
			weight[top] = N_ADJ - backtracks;
			backtracks++;
			next_hop();
		}
	}
}
