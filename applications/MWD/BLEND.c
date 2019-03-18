#include <api.h>
#include <stdlib.h>

Message msg;

int main()
{

int i, j;

	for(j=0;j<128;j++) msg.msg[j]=i;

	/*Comm SE 640*/
	msg.length=128;
	for(j=0;j<5;j++) Receive(&msg,SE);

exit();

}
