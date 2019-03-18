#include <api.h>
#include <stdlib.h>

Message msg;

int main()
{

int i, j;

	for(j=0;j<128;j++) msg.msg[j]=i;

	/*Comm VS 960*/
	msg.length=128;
	for(j=0;j<7;j++) Receive(&msg,VS);
	msg.length=64;
	Receive(&msg,VS);
	/*Comm MEM3 960*/
	msg.length=128;
	for(j=0;j<7;j++) Send(&msg,MEM3);
	msg.length=64;
	Send(&msg,MEM3);

exit();

}
