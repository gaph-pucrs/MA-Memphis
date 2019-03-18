#include <api.h>
#include <stdlib.h>

Message msg;

int main()
{

int i, j;

	for(j=0;j<128;j++) msg.msg[j]=i;

	/*Comm IN 1280*/
	msg.length=128;
	for(j=0;j<10;j++) Receive(&msg,IN);
	/*Comm VS 960*/
	msg.length=128;
	for(j=0;j<7;j++) Send(&msg,VS);
	msg.length=64;
	Send(&msg,VS);

exit();

}
