#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>

int msg[128];

int main()
{

int j;

	for(j=0;j<128;j++) msg[j]=j;

	/*Comm NR 960*/
	for(j=0;j<7;j++) memphis_receive(msg, sizeof(msg), NR);
	memphis_receive(msg, 64*sizeof(int), NR);
	/*Comm HVS 960*/
	for(j=0;j<7;j++) memphis_send(msg, sizeof(msg), HVS);
	memphis_send(msg, 64*sizeof(int), HVS);

return 0;

}
