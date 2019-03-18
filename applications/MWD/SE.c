#include <api.h>
#include <stdlib.h>

Message msg;

int main()
{

int i, j;

	for(j=0;j<128;j++) msg.msg[j]=i;

	/*Comm MEM3 640*/
	msg.length=128;
	for(j=0;j<5;j++) Receive(&msg,MEM3);
	/*Comm BLEND 640*/
	msg.length=128;
	for(j=0;j<5;j++) Send(&msg,BLEND);

exit();

}
