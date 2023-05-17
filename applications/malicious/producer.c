/**
 * MA-Memphis
 * @file producer.c
 * 
 * @author Angelo Elias Dal Zotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date February 2023
 * 
 * @brief Producer task of a malicious application 
 */

#include <stdlib.h>
#include <stdio.h>

#include <memphis.h>

#include "config.h"

int main()
{
	int *msg = malloc(MAL_MSG_SIZE*sizeof(int));

	unsigned then = -MAL_TICK_INTERVAL;

	for(int i = 0; i < MAL_ITER; i++){
		while(memphis_get_tick() - then < MAL_TICK_INTERVAL);
		memphis_send(msg, MAL_MSG_SIZE*sizeof(int), consumer);
		then = memphis_get_tick();
	}
	
	return 0;
}
