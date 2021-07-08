#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>

message_t msg;

int main()
{

int j;

	for(j=0;j<128;j++) msg.payload[j]=j;

	/*Comm JUG1 960*/
	msg.length=128;
	for(j=0;j<7;j++) memphis_receive(&msg,JUG1);
	msg.length=64;
	memphis_receive(&msg,JUG1);
	/*Comm JUG2 960*/
	msg.length=128;
	for(j=0;j<7;j++) memphis_receive(&msg,JUG2);
	msg.length=64;
	memphis_receive(&msg,JUG2);
	/*Comm SE 640*/
	msg.length=128;
	for(j=0;j<5;j++) memphis_send(&msg,SE);

return 0;

}
