#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>

int main()
{

int msg[128];
int j;

	for(j=0;j<128;j++) msg[j]=j;

	for(j=0;j<8;j++) memphis_receive(msg, 128*sizeof(int), ACDC_0);
	memphis_receive(msg, 88*sizeof(int), ACDC_0);
	memphis_receive(msg, 128*sizeof(int), STRIPEM_0);
	for(j=0;j<8;j++) memphis_send(msg, 128*sizeof(int), IDCT2_0);
	memphis_send(msg, 72*sizeof(int), IDCT2_0);

	return 0;

}
