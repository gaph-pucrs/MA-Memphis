#include <api.h>
#include <stdlib.h>

Message msg;

int main()
{

int i, j;

	for(j=0;j<128;j++) msg.msg[j]=i;

	/*Comm HS 960*/
	msg.length=128;
	for(j=0;j<7;j++) Receive(&msg,HS);
	msg.length=64;
	Receive(&msg,HS);
	/*Comm JUG1 960*/
	msg.length=128;
	for(j=0;j<7;j++) Send(&msg,JUG1);
	msg.length=64;
	Send(&msg,JUG1);

exit();

}
