#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>

int msg[128];

int main()
{

int j;

	for(j=0;j<128;j++) msg[j]=j;

	/*Comm JUG1 960*/
	for(j=0;j<7;j++) memphis_receive(msg, sizeof(msg), JUG1);
	memphis_receive(msg, 64*sizeof(int), JUG1);
	/*Comm JUG2 960*/
	for(j=0;j<7;j++) memphis_receive(msg, sizeof(msg), JUG2);
	memphis_receive(msg, 64*sizeof(int), JUG2);
	/*Comm SE 640*/
	for(j=0;j<5;j++) memphis_send(msg, sizeof(msg), SE);

return 0;

}
