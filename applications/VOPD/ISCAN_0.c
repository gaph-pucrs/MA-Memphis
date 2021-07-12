#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>

int main()
{

message_t msg;
int j;

	for(j=0;j<128;j++) msg.payload[j]=j;

	msg.length=128;
	for(j=0;j<8;j++) memphis_receive(&msg,RUN_0);
	msg.length=88;
	memphis_receive(&msg,RUN_0);
	msg.length=128;
	for(j=0;j<8;j++) memphis_send(&msg,ACDC_0);
	msg.length=88;
	memphis_send(&msg,ACDC_0);

	puts("Chamou exit\n");

	exit(0);

}
