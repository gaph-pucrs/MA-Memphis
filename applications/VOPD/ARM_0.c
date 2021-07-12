#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>

int main()
{

message_t msg;
int j;

	for(j=0;j<128;j++) msg.payload[j]=j;

	msg.length=128;
	memphis_send(&msg,IDCT2_0);
	
	return 0;

}
