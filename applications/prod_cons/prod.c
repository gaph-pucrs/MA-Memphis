/*
 * prod.c
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
	volatile int t;


	Echo("Inicio da aplicacao prod");

	for(i=0;i<MSG_SIZE;i++) msg.msg[i]=i;
	msg.length = MSG_SIZE;

	msg.msg[9]=0xBA;

	for(i=0; i<PROD_CONS_ITERATIONS; i++){
		Send(&msg, cons);
	}


	Echo("Fim da aplicacao prod");
	exit();

}


