#include <api.h>
#include <stdlib.h>

int main()
{

int j;
Message msg;

//Echo(strcat("b,SRAM1,",itoa(GetTick())));


	for(j=0;j<128;j++) msg.msg[j]=j;

	/*Comm MCPU 1280*/
	msg.length=128;
	for(j=0;j<10;j++) Receive(&msg,MCPU_0);
	//Echo(strcat("r,MPEG_m14(1280),",itoa(GetTick())));
	/*Comm RAST 1280*/
	msg.length=128;
	for(j=0;j<10;j++) Send(&msg,RAST_0);
	//Echo(strcat("s,MPEG_m15(1280),",itoa(GetTick())));
	//Echo(strcat(strcat(strcat("i,",itoa(i)),","),itoa(GetTick())));

//Echo(strcat("e,SRAM1,",itoa(GetTick())));

exit();

}
