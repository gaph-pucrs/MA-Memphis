#include <api.h>
#include <stdlib.h>

int main()
{

Message msg;
int j;

	for(j=0;j<128;j++) msg.msg[j]=j;

	msg.length=128;
	for(j=0;j<8;j++) Receive(&msg,ISCAN_0);
	msg.length=88;
	Receive(&msg,ISCAN_0);
	msg.length=128;
	Send(&msg,STRIPEM_0);
	msg.length=22;
	Send(&msg,STRIPEM_0);
	msg.length=128;
	for(j=0;j<8;j++) Send(&msg,IQUANT_0);
	msg.length=88;
	Send(&msg,IQUANT_0);

exit();

}
