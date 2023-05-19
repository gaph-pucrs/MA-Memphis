#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>

int msg[128];

int main()
{

int j;

	for(j=0;j<128;j++) msg[j]=j;

	/*Comm HS 960*/
	for(j=0;j<7;j++) memphis_receive(msg, sizeof(msg), HS);
	memphis_receive(msg, 64*sizeof(int), HS);
	/*Comm JUG1 960*/
	for(j=0;j<7;j++) memphis_send(msg, sizeof(msg), JUG1);
	memphis_send(msg, 64*sizeof(int), JUG1);

return 0;

}
