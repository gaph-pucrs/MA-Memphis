#include <api.h>
#include <stdlib.h>

Message msg;

int main()
{

int i, j;

	for(j=0;j<128;j++) msg.msg[j]=i;

	/*Comm JUG1 960*/
	msg.length=128;
	for(j=0;j<7;j++) Receive(&msg,JUG1);
	msg.length=64;
	Receive(&msg,JUG1);
	/*Comm JUG2 960*/
	msg.length=128;
	for(j=0;j<7;j++) Receive(&msg,JUG2);
	msg.length=64;
	Receive(&msg,JUG2);
	/*Comm SE 640*/
	msg.length=128;
	for(j=0;j<5;j++) Send(&msg,SE);

exit();

}
