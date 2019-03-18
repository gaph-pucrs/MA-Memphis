#include <api.h>
#include <stdlib.h>

int main()
{

Message msg;
int j;

	for(j=0;j<128;j++) msg.msg[j]=j;

	msg.length=128;
	for(j=0;j<7;j++) Receive(&msg,PAD_0);
	msg.length=66;
	Receive(&msg,PAD_0);
	msg.length=128;
	for(j=0;j<2;j++) Send(&msg,PAD_0);
	msg.length=33;
	Send(&msg,PAD_0);
	msg.length=128;
	for(j=0;j<11;j++) Send(&msg,VOPREC_0);
	msg.length=92;
	Send(&msg,VOPREC_0);

	exit();

}
