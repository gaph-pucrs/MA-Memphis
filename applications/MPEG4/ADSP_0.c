#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>

int main()
{

int j;
int msg[128];

//Echo(strcat("b,ADSP,",itoa(GetTick())));


	for(j=0;j<128;j++) msg[j]=j;

	/*Comm SDRAM 1280*/
	for(j=0;j<10;j++) memphis_receive(msg, sizeof(msg), SDRAM_0);
	//Echo(strcat("r,MPEG_m11(1280),",itoa(GetTick())));
	/*Comm SDRAM 1280*/
	for(j=0;j<10;j++) memphis_send(msg, sizeof(msg), SDRAM_0);
	//Echo(strcat("s,MPEG_m13(1280),",itoa(GetTick())));
	//Echo(strcat(strcat(strcat("i,",itoa(i)),","),itoa(GetTick())));

//Echo(strcat("e,ADSP,",itoa(GetTick())));

return 0;

}
