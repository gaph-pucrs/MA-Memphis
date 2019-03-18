#include <api.h>
#include <stdlib.h>

int main()
{

int j;
Message msg;

//Echo(strcat("b,MCPU,",itoa(GetTick())));


	for(j=0;j<128;j++) msg.msg[j]=j;

	/*Comm SDRAM 1280*/
	msg.length=128;
	for(j=0;j<10;j++) Receive(&msg,SDRAM_0);
	//Echo(strcat("r,MPEG_m12(1280),",itoa(GetTick())));
	/*Comm SRAM1 1280*/
	msg.length=128;
	for(j=0;j<10;j++) Send(&msg,SRAM1_0);
	//Echo(strcat("s,MPEG_m14(1280),",itoa(GetTick())));
	//Echo(strcat(strcat(strcat("i,",itoa(i)),","),itoa(GetTick())));

//Echo(strcat("e,MCPU,",itoa(GetTick())));

exit();

}
