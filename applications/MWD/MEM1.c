#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>

int msg[128];

int main()
{

int j;

	for(j=0;j<128;j++) msg[j]=j;

	/*Comm NR 640*/
	for(j=0;j<5;j++) memphis_receive(msg, sizeof(msg), NR);
	/*Comm NR 640*/
	for(j=0;j<5;j++) memphis_send(msg, sizeof(msg), NR);

return 0;

}
