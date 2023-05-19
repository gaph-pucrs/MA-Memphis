#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>

int msg[128];

int main()
{

int j;

	for(j=0;j<128;j++) msg[j]=j;

	/*Comm MEM2 960*/
	for(j=0;j<7;j++) memphis_receive(msg, sizeof(msg), MEM2);
	memphis_receive(msg, 64*sizeof(int), MEM2);
	/*Comm JUG2 960*/
	for(j=0;j<7;j++) memphis_send(msg, sizeof(msg), JUG2);
	memphis_send(msg, 64*sizeof(int), JUG2);

return 0;

}
