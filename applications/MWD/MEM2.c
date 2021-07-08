#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>

message_t msg;

int main()
{

int j;

	for(j=0;j<128;j++) msg.payload[j]=j;

	/*Comm NR 960*/
	msg.length=128;
	for(j=0;j<7;j++) memphis_receive(&msg,NR);
	msg.length=64;
	memphis_receive(&msg,NR);
	/*Comm HVS 960*/
	msg.length=128;
	for(j=0;j<7;j++) memphis_send(&msg,HVS);
	msg.length=64;
	memphis_send(&msg,HVS);

return 0;

}
