#include <api.h>
#include <stdlib.h>

Message msg;

int main()
{

int i, j;

	for(j=0;j<128;j++) msg.msg[j]=i;

	/*Comm MEM2 960*/
	msg.length=128;
	for(j=0;j<7;j++) Receive(&msg,MEM2);
	msg.length=64;
	Receive(&msg,MEM2);
	/*Comm JUG2 960*/
	msg.length=128;
	for(j=0;j<7;j++) Send(&msg,JUG2);
	msg.length=64;
	Send(&msg,JUG2);

exit();

}
