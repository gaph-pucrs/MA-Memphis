#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>

int msg[128];

int main()
{

int j;

	for(j=0;j<128;j++) msg[j]=j;

	/*Comm HS 1280*/
	for(j=0;j<10;j++) memphis_send(msg, sizeof(msg), HS);
	/*Comm NR 640*/
	for(j=0;j<5;j++) memphis_send(msg, sizeof(msg), NR);

return 0;

}
