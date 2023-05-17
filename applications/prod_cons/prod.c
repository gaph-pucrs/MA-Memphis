/**
 * MA-Memphis
 * @file prod.c
 * 
 * @author Marcelo Ruaro (marcelo.ruaro@acad.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date March 2013
 * 
 * @brief Producer task of a Producer-Consumer test application 
 */

#include <stdlib.h>
#include <stdio.h>

#include <memphis.h>

#include "prod_cons_std.h"

int main()
{
	puts("Inicio da aplicacao prod");

	int *msg = malloc(PROD_CONS_MSG_SIZE*sizeof(int));

	for(int i = 0; i < PROD_CONS_MSG_SIZE; i++) 
		msg[i] = i;

	for(int i = 0; i < PROD_CONS_ITERATIONS; i++){
		memphis_send(msg, PROD_CONS_MSG_SIZE*sizeof(int), cons);
	}

	puts("Fim da aplicacao prod");
	return 0;
}
