/*
 * cons.c
 *
 *  Created on: 07/03/2013
 *      Author: mruaro
 */

#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>
#include "prod_cons_std.h"

message_t msg;

int main()
{
	int i;
	unsigned int time[PROD_CONS_ITERATIONS];

	printf("Inicio da aplicacao cons %d\n", PROD_CONS_ITERATIONS);

	for(i=0; i<PROD_CONS_ITERATIONS; i++){
		memphis_receive(&msg, prod);
		time[i] = memphis_get_tick();
        printf("Rcv %d\n", i);
	}


	for(i=0; i<PROD_CONS_ITERATIONS; i++){
		printf("%d\n",time[i]);
	}

	puts("Fim da aplicacao cons\n");

	return 0;
}
