#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>

int main()
{

message_t msg;
int j;

	for(j=0;j<128;j++) msg.payload[j]=j;

	msg.length=128;
	for(j=0;j<7;j++) memphis_receive(&msg,UPSAMP_0);
	msg.length=26;
	memphis_receive(&msg,UPSAMP_0);
	msg.length=128;
	for(j=0;j<7;j++) memphis_send(&msg,PAD_0);
	msg.length=66;
	memphis_send(&msg,PAD_0);
	msg.length=128;
	for(j=0;j<11;j++) memphis_receive(&msg,VOPME_0);
	msg.length=92;
	memphis_receive(&msg,VOPME_0);

	return 0;

}
