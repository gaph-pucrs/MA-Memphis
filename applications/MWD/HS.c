#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>

message_t msg;

int main()
{

int j;

	for(j=0;j<128;j++) msg.payload[j]=j;

	/*Comm IN 1280*/
	msg.length=128;
	for(j=0;j<10;j++) memphis_receive(&msg,IN);
	/*Comm VS 960*/
	msg.length=128;
	for(j=0;j<7;j++) memphis_send(&msg,VS);
	msg.length=64;
	memphis_send(&msg,VS);

return 0;

}
