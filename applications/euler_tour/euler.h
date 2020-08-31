/**
 * Euler tour application for Memphis
 * 
 * @file euler.h
 * 
 * @author Angelo Elias Dalzotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @brief This file models the graph and defines the application level message
 * protocol
 * 
 * @date June 2018
 */

#include <stdbool.h>
#include <stdlib.h>

#define DEBUG 1

#define NODE_M 0
#define NODE_A 1
#define NODE_B 2
#define NODE_C 3

/* Graph model */
/* THE GRAPH MUST BE CONNECTED */

#define TOTAL_EDGES 3
#define TOTAL_TASKS 4
const int P[TOTAL_TASKS] = {manager, node_a, node_b, node_c};

// Node degree. First always 0
const int deg[TOTAL_TASKS] = {0, 2, 2, 2};

const char *start_msg[TOTAL_TASKS] = {"Start M",
									"Start A",
									"Start B",
									"Start C"};
const char *exit_msg[TOTAL_TASKS] = {"Exit M",
									"Exit A",
									"Exit B",
									"Exit C"};
const char *rcv_msg[TOTAL_TASKS] = {"Rcv M",
								"Rcv A",
								"Rcv B",
								"Rcv C"};
const char *bkt_msg[TOTAL_TASKS] = {"Bkt M",
									"Bkt A",
									"Bkt B",
									"Bkt C"};
const char *fwd_msg[TOTAL_TASKS] = {"Fwd M",
										"Fwd A",
										"Fwd B",
										"Fwd C"};

									// m, a, b, c
const int ntoa[TOTAL_TASKS][TOTAL_TASKS] = {{0, 0, 0, 0}, //m
									{0, 0, 0, 1},   //a
									{0, 0, 0, 1},   //b
									{0, 0, 1, 0}};   //c

const int atop[TOTAL_TASKS][TOTAL_TASKS] = {{0, 0, 0, 0},
											{2, 3, 0, 0},
											{1, 3, 0, 0},
											{1, 2, 0, 0}};


// DONT CHANGE
#define LENGHT 4
#define MSG_OP msg.msg[0]
#define MSG_SRC msg.msg[1]
#define TUNUE msg.msg[2]
#define BK_ADDR msg.msg[3]

#define FORWARD 1
#define BACKTRACK -1
#define EXIT 0
