#include <api.h>
#include <stdlib.h>

Message msg;

int main()
{

int i, j;

	for(j=0;j<128;j++) msg.msg[j]=i;

	/*Comm IN 640*/
	msg.length=128;
	for(j=0;j<5;j++) Receive(&msg,IN);
	/*Comm MEM1 640*/
	msg.length=128;
	for(j=0;j<5;j++) Send(&msg,MEM1);
	/*Comm MEM1 640*/
	msg.length=128;
	for(j=0;j<5;j++) Receive(&msg,MEM1);
	/*Comm MEM2 960*/
	msg.length=128;
	for(j=0;j<7;j++) Send(&msg,MEM2);
	msg.length=64;
	Send(&msg,MEM2);

exit();

}
