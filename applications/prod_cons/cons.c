/**
 * MA-Memphis
 * @file cons.c
 * 
 * @author Marcelo Ruaro (marcelo.ruaro@acad.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date March 2013
 * 
 * @brief Consumer task of a Producer-Consumer test application 
 */

#include <stdlib.h>
#include <stdio.h>

#include <memphis.h>

#include "prod_cons_std.h"

int main()
{
	unsigned *time = malloc(PROD_CONS_ITERATIONS*sizeof(unsigned));
	int *msg = malloc(PROD_CONS_MSG_SIZE*sizeof(int));

	puts("Inicio da aplicacao cons");

	for(int i = 0; i < PROD_CONS_ITERATIONS; i++){
		memphis_receive(msg, PROD_CONS_MSG_SIZE*sizeof(int), prod);
		time[i] = memphis_get_tick();
	}

	for(int i = 0; i < PROD_CONS_ITERATIONS; i++){
		printf("%d\n", time[i]);
	}

	puts("Fim da aplicacao cons");

	return 0;
}
