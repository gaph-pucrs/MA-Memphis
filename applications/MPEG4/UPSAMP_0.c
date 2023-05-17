#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>

int main()
{

int j;
int msg[128];

//Echo(strcat("b,UPSAMP,",itoa(GetTick())));


	for(j=0;j<128;j++) msg[j]=j;

	/*Comm SRAM2 11310*/
	for(j=0;j<88;j++) memphis_receive(msg, sizeof(msg), SRAM2_0);
	//Echo("FIM RECEIVEEEEEEEEEEEEEEEEEEEEE!!!!!!!!!!!!!\n");
	memphis_receive(msg, 46*sizeof(int), SRAM2_0);
	//Echo("FIM RECEIVEEEEEEEEEEEEEEEEEEEEE!!!!!!!!!!!!!\n");
	//Echo(strcat("r,MPEG_m7(11310),",itoa(GetTick())));
	/*Comm SDRAM 15000*/
	for(j=0;j<117;j++)
	{
		//Echo("ANTES!!!!!!!!!!!!!\n");
		memphis_receive(msg, sizeof(msg), SDRAM_0);
 		//Echo("DEPOIS!!!!!!!!!!!!!\n");
	}
	memphis_receive(msg, 24*sizeof(int), SDRAM_0);
	//Echo(strcat("r,MPEG_m9(15000),",itoa(GetTick())));
	/*Comm SDRAM 15000*/
	for(j=0;j<117;j++) memphis_send(msg, sizeof(msg), SDRAM_0);
	memphis_send(msg, 24*sizeof(int), SDRAM_0);
	//Echo(strcat("s,MPEG_m10(15000),",itoa(GetTick())));
	//Echo(strcat(strcat(strcat("i,",itoa(i)),","),itoa(GetTick())));

//Echo(strcat("e,UPSAMP,",itoa(GetTick())));

return 0;

}
