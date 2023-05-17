#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>

int msg[128];

int main()
{

int j;

	for(j=0;j<128;j++) msg[j]=j;

	/*Comm IN 1280*/
	for(j=0;j<10;j++) memphis_receive(msg, sizeof(msg), IN);
	/*Comm VS 960*/
	for(j=0;j<7;j++) memphis_send(msg, sizeof(msg), VS);
	memphis_send(msg, 64*sizeof(int), VS);

return 0;

}
