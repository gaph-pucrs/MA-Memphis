#include <api.h>
#include <stdlib.h>

Message msg;

int main()
{

int j;

	for(j=0;j<128;j++) msg.msg[j]=j;

	/*Comm SE 640*/
	msg.length=128;
	for(j=0;j<5;j++) Receive(&msg,SE);

return 0;

}
