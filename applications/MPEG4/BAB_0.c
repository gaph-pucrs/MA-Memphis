#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>

int main()
{

int j;
int msg[128];

//Echo(strcat("b,BAB,",itoa(GetTick())));


	for(j=0;j<128;j++) msg[j]=j;

	/*Comm SRAM2 2930*/
	for(j=0;j<22;j++) memphis_receive(msg, sizeof(msg), SRAM2_0);
	memphis_receive(msg, 114*sizeof(int), SRAM2_0);
	//Echo(strcat("r,MPEG_m3(2930),",itoa(GetTick())));
	/*Comm SDRAM 1280*/
	for(j=0;j<10;j++) memphis_receive(msg, sizeof(msg), SDRAM_0);
	//Echo(strcat("r,MPEG_m6(1280),",itoa(GetTick())));
	/*Comm SDRAM 1280*/
	for(j=0;j<10;j++) memphis_send(msg, sizeof(msg), SDRAM_0);
	//Echo(strcat("s,MPEG_m8(1280),",itoa(GetTick())));
	//Echo(strcat(strcat(strcat("i,",itoa(i)),","),itoa(GetTick())));

//Echo(strcat("e,BAB,",itoa(GetTick())));

return 0;

}
