#include <api.h>
#include <stdlib.h>

int main()
{

Message msg;
int j;

	for(j=0;j<128;j++) msg.msg[j]=j;

	msg.length=128;
	for(j=0;j<8;j++) Receive(&msg,ACDC_0);
	msg.length=88;
	Receive(&msg,ACDC_0);
	msg.length=128;
	Receive(&msg,STRIPEM_0);
	msg.length=128;
	for(j=0;j<8;j++) Send(&msg,IDCT2_0);
	msg.length=72;
	Send(&msg,IDCT2_0);

	exit();

}
