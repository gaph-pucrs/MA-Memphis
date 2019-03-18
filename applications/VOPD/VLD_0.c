#include <api.h>
#include <stdlib.h>

int main()
{

Message msg;
int j;

	for(j=0;j<128;j++) msg.msg[j]=j;

	msg.length=128;
	Send(&msg,RUN_0);
	msg.length=87;
	Send(&msg,RUN_0);

	exit();

}
