#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>

int main()
{

int j;
int msg[128];

//Echo(strcat("b,MCPU,",itoa(GetTick())));


	for(j=0;j<128;j++) msg[j]=j;

	/*Comm SDRAM 1280*/
	for(j=0;j<10;j++) memphis_receive(msg, sizeof(msg), SDRAM_0);
	//Echo(strcat("r,MPEG_m12(1280),",itoa(GetTick())));
	/*Comm SRAM1 1280*/
	for(j=0;j<10;j++) memphis_send(msg, sizeof(msg), SRAM1_0);
	//Echo(strcat("s,MPEG_m14(1280),",itoa(GetTick())));
	//Echo(strcat(strcat(strcat("i,",itoa(i)),","),itoa(GetTick())));

//Echo(strcat("e,MCPU,",itoa(GetTick())));

return 0;

}
