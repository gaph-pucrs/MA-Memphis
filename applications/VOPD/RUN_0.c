#include <api.h>
#include <stdlib.h>

int main()
{

Message msg;
int j;

	for(j=0;j<128;j++) msg.msg[j]=j;

	msg.length=128;
	Receive(&msg,VLD_0);
	msg.length=87;
	Receive(&msg,VLD_0);
	msg.length=128;
	for(j=0;j<8;j++) Send(&msg,ISCAN_0);
	msg.length=88;
	Send(&msg,ISCAN_0);

	exit();

}
