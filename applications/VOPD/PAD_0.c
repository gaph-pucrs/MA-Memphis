#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>

int main()
{

int msg[128];
int j;

	for(j=0;j<128;j++) msg[j]=j;

	for(j=0;j<7;j++) memphis_receive(msg, 128*sizeof(int), VOPREC_0);
	memphis_receive(msg, 66*sizeof(int), VOPREC_0);
	for(j=0;j<7;j++) memphis_send(msg, 128*sizeof(int), VOPME_0);
	memphis_send(msg, 66*sizeof(int), VOPME_0);
	for(j=0;j<2;j++) memphis_receive(msg, 128*sizeof(int), VOPME_0);
	memphis_receive(msg, 33*sizeof(int), VOPME_0);

	return 0;

}
