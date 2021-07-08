#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>

int main()
{

int j;
message_t msg;

//Echo(strcat("b,RAST,",itoa(GetTick())));


	for(j=0;j<128;j++) msg.payload[j]=j;

	/*Comm SRAM1 1280*/
	msg.length=128;
	for(j=0;j<10;j++) memphis_receive(&msg,SRAM1_0);
	//Echo(strcat("r,MPEG_m15(1280),",itoa(GetTick())));
	/*Comm SDRAM 10130*/
	msg.length=128;
	for(j=0;j<79;j++) memphis_receive(&msg,SDRAM_0);
	msg.length=18;
	memphis_receive(&msg,SDRAM_0);
	//Echo(strcat("r,MPEG_m16(10130),",itoa(GetTick())));
	/*Comm SDRAM 10130*/
	msg.length=128;
	for(j=0;j<79;j++) memphis_send(&msg,SDRAM_0);
	msg.length=18;
	memphis_send(&msg,SDRAM_0);
	//Echo(strcat("s,MPEG_m17(10130),",itoa(GetTick())));
	//Echo(strcat(strcat(strcat("i,",itoa(i)),","),itoa(GetTick())));

//Echo(strcat("e,RAST,",itoa(GetTick())));

return 0;

}
