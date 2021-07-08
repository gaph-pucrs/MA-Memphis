#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>

int main()
{

int j;
message_t msg;

//Echo(strcat("b,UPSAMP,",itoa(GetTick())));


	for(j=0;j<128;j++) msg.payload[j]=j;

	/*Comm SRAM2 11310*/
	msg.length=128;
	for(j=0;j<88;j++) memphis_receive(&msg,SRAM2_0);
	//Echo("FIM RECEIVEEEEEEEEEEEEEEEEEEEEE!!!!!!!!!!!!!\n");
	msg.length=46;
	memphis_receive(&msg,SRAM2_0);
	//Echo("FIM RECEIVEEEEEEEEEEEEEEEEEEEEE!!!!!!!!!!!!!\n");
	//Echo(strcat("r,MPEG_m7(11310),",itoa(GetTick())));
	/*Comm SDRAM 15000*/
	msg.length=128;
	for(j=0;j<117;j++)
	{
		//Echo("ANTES!!!!!!!!!!!!!\n");
		memphis_receive(&msg,SDRAM_0);
 		//Echo("DEPOIS!!!!!!!!!!!!!\n");
	}
	msg.length=24;
	memphis_receive(&msg,SDRAM_0);
	//Echo(strcat("r,MPEG_m9(15000),",itoa(GetTick())));
	/*Comm SDRAM 15000*/
	msg.length=128;
	for(j=0;j<117;j++) memphis_send(&msg,SDRAM_0);
	msg.length=24;
	memphis_send(&msg,SDRAM_0);
	//Echo(strcat("s,MPEG_m10(15000),",itoa(GetTick())));
	//Echo(strcat(strcat(strcat("i,",itoa(i)),","),itoa(GetTick())));

//Echo(strcat("e,UPSAMP,",itoa(GetTick())));

return 0;

}
