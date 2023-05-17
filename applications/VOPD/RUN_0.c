#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>

int main()
{

int msg[128];
int j;

	for(j=0;j<128;j++) msg[j]=j;

	memphis_receive(msg, 128*sizeof(int), VLD_0);
	memphis_receive(msg, 87*sizeof(int), VLD_0);
	for(j=0;j<8;j++) memphis_send(msg, 128*sizeof(int), ISCAN_0);
	memphis_send(msg, 88*sizeof(int), ISCAN_0);

	return 0;

}
