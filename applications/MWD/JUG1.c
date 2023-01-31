#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>

int msg[128];

int main()
{

int j;

	for(j=0;j<128;j++) msg[j]=j;

	/*Comm VS 960*/
	for(j=0;j<7;j++) memphis_receive(msg, sizeof(msg), VS);
	memphis_receive(msg, 64*sizeof(int), VS);
	/*Comm MEM3 960*/
	for(j=0;j<7;j++) memphis_send(msg, sizeof(msg), MEM3);
	memphis_send(msg, 64*sizeof(int), MEM3);

return 0;

}
