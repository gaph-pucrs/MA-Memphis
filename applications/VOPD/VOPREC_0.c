#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>

int main()
{

int msg[128];
int j;

	for(j=0;j<128;j++) msg[j]=j;

	for(j=0;j<7;j++) memphis_receive(msg, 128*sizeof(int), UPSAMP_0);
	memphis_receive(msg, 26*sizeof(int), UPSAMP_0);
	for(j=0;j<7;j++) memphis_send(msg, 128*sizeof(int), PAD_0);
	memphis_send(msg, 66*sizeof(int), PAD_0);
	for(j=0;j<11;j++) memphis_receive(msg, 128*sizeof(int), VOPME_0);
	memphis_receive(msg, 92*sizeof(int), VOPME_0);

	return 0;

}
