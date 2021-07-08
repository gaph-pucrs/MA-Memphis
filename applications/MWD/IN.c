#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>

message_t msg;

int main()
{

int j;

	for(j=0;j<128;j++) msg.payload[j]=j;

	/*Comm HS 1280*/
	msg.length=128;
	for(j=0;j<10;j++) memphis_send(&msg,HS);
	/*Comm NR 640*/
	msg.length=128;
	for(j=0;j<5;j++) memphis_send(&msg,NR);

return 0;

}
