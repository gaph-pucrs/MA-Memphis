/**
 * MA-Memphis
 * @file consumer.c
 * 
 * @author Angelo Elias Dal Zotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date February 2023
 * 
 * @brief Consumer task of a malicious application 
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <memphis.h>

#include "config.h"

int main()
{
	int *msg = malloc(MAL_MSG_SIZE*sizeof(int));

	while(true){
		memphis_receive(msg, MAL_MSG_SIZE*sizeof(int), producer);
		if(msg[0] == 0)
			break;
	}

	return 0;
}
