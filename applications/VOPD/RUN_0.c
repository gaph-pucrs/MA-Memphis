#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>

int main()
{

message_t msg;
int j;

	for(j=0;j<128;j++) msg.payload[j]=j;

	msg.length=128;
	memphis_receive(&msg,VLD_0);
	msg.length=87;
	memphis_receive(&msg,VLD_0);
	msg.length=128;
	for(j=0;j<8;j++) memphis_send(&msg,ISCAN_0);
	msg.length=88;
	memphis_send(&msg,ISCAN_0);

	return 0;

}
