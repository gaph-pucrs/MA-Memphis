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
	msg[0] = 1;

	while(memphis_get_tick() < MAL_TRIGGER_START); /* @todo memphis_yield() to yield its slack time to scheduler */

	uint32_t then = 0;
	uint32_t now;
	do {
		do {
			now = memphis_get_tick();
		} while((now - then) < MAL_MSG_INTERVAL);
		then = now;
		memphis_send(msg, MAL_MSG_SIZE*sizeof(int), consumer);
	} while(memphis_get_tick() < MAL_TRIGGER_END);

	msg[0] = 0;
	memphis_send(msg, sizeof(int), consumer);
	
	return 0;
}
