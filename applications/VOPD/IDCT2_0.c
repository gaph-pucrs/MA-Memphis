#include <api.h>
#include <stdlib.h>

int main()
{

Message msg;
int j;

	for(j=0;j<128;j++) msg.msg[j]=j;

	msg.length=128;
	Receive(&msg,ARM_0);
	msg.length=128;
	for(j=0;j<8;j++) Receive(&msg,IQUANT_0);
	msg.length=72;
	Receive(&msg,IQUANT_0);
	msg.length=128;
	for(j=0;j<8;j++) Send(&msg,UPSAMP_0);
	msg.length=61;
	Send(&msg,UPSAMP_0);

	exit();

}
