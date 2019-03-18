#include <api.h>
#include <stdlib.h>

int main()
{

Message msg;
int j;

	for(j=0;j<128;j++) msg.msg[j]=j;

	msg.length=128;
	Receive(&msg,ACDC_0);
	msg.length=22;
	Receive(&msg,ACDC_0);
	msg.length=128;
	Send(&msg,IQUANT_0);

	exit();

}
