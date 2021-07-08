#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>

message_t msg;

int main()
{

int j;

	for(j=0;j<128;j++) msg.payload[j]=j;

	/*Comm HVS 960*/
	msg.length=128;
	for(j=0;j<7;j++) memphis_receive(&msg,HVS);
	msg.length=64;
	memphis_receive(&msg,HVS);
	/*Comm MEM3 960*/
	msg.length=128;
	for(j=0;j<7;j++) memphis_send(&msg,MEM3);
	msg.length=64;
	memphis_send(&msg,MEM3);

return 0;

}
