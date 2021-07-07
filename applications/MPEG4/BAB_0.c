#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>

int main()
{

int j;
message_t msg;

//Echo(strcat("b,BAB,",itoa(GetTick())));


	for(j=0;j<128;j++) msg.payload[j]=j;

	/*Comm SRAM2 2930*/
	msg.length=128;
	for(j=0;j<22;j++) memphis_receive(&msg,SRAM2_0);
	msg.length=114;
	memphis_receive(&msg,SRAM2_0);
	//Echo(strcat("r,MPEG_m3(2930),",itoa(GetTick())));
	/*Comm SDRAM 1280*/
	msg.length=128;
	for(j=0;j<10;j++) memphis_receive(&msg,SDRAM_0);
	//Echo(strcat("r,MPEG_m6(1280),",itoa(GetTick())));
	/*Comm SDRAM 1280*/
	msg.length=128;
	for(j=0;j<10;j++) memphis_send(&msg,SDRAM_0);
	//Echo(strcat("s,MPEG_m8(1280),",itoa(GetTick())));
	//Echo(strcat(strcat(strcat("i,",itoa(i)),","),itoa(GetTick())));

//Echo(strcat("e,BAB,",itoa(GetTick())));

return 0;

}
