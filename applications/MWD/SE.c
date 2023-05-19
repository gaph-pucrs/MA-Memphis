#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>

int msg[128];

int main()
{

int j;

	for(j=0;j<128;j++) msg[j]=j;

	/*Comm MEM3 640*/
	for(j=0;j<5;j++) memphis_receive(msg, sizeof(msg), MEM3);
	/*Comm BLEND 640*/
	for(j=0;j<5;j++) memphis_send(msg, sizeof(msg), BLEND);

return 0;

}
