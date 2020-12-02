#include <api.h>
#include <stdlib.h>

Message msg;

int main()
{

int j;

	for(j=0;j<128;j++) msg.msg[j]=j;

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

return 0;

}
