#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>

message_t msg;

int main()
{

int j;

	for(j=0;j<128;j++) msg.payload[j]=j;

	/*Comm IN 640*/
	msg.length=128;
	for(j=0;j<5;j++) memphis_receive(&msg,IN);
	/*Comm MEM1 640*/
	msg.length=128;
	for(j=0;j<5;j++) memphis_send(&msg,MEM1);
	/*Comm MEM1 640*/
	msg.length=128;
	for(j=0;j<5;j++) memphis_receive(&msg,MEM1);
	/*Comm MEM2 960*/
	msg.length=128;
	for(j=0;j<7;j++) memphis_send(&msg,MEM2);
	msg.length=64;
	memphis_send(&msg,MEM2);

return 0;

}
