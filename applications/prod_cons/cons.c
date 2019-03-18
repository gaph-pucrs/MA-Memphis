/*
 * cons.c
 *
 *  Created on: 07/03/2013
 *      Author: mruaro
 */

#include <api.h>
#include <stdlib.h>
#include "prod_cons_std.h"


Message msg;

int main()
{

	int i;
	volatile int p;
	unsigned int time[PROD_CONS_ITERATIONS];

	Echo("Inicio da aplicacao cons");

	for(i=0; i<PROD_CONS_ITERATIONS; i++){

		Receive(&msg, prod);
		time[i] = GetTick();
	}


	for(i=0; i<PROD_CONS_ITERATIONS; i++){
		Echo(itoa(time[i]));
	}

	Echo("Fim da aplicacao cons");

	exit();

}


