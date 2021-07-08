/*
 * prod.c
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

	puts("Inicio da aplicacao prod\n");

	for(i=0;i<PKG_MAX_MSG_SIZE;i++) msg.payload[i]=i;
	msg.length = PKG_MAX_MSG_SIZE;

	msg.payload[9]=0xBA;

	for(i=0; i<PROD_CONS_ITERATIONS; i++){
		memphis_send(&msg, cons);
	}

	puts("Fim da aplicacao prod\n");
	return 0;
}
