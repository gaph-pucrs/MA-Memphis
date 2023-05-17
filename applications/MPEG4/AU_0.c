#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>

int main()
{

int j;
int msg[128];

//Echo(strcat("b,AU,",itoa(GetTick())));


	for(j=0;j<128;j++) msg[j]=j;

	/*Comm SDRAM 1280*/
	for(j=0;j<10;j++) memphis_receive(msg, sizeof(msg),SDRAM_0);
	//Echo(strcat("r,MPEG_m18(1280),",itoa(GetTick())));
	//Echo(strcat(strcat(strcat("i,",itoa(i)),","),itoa(GetTick())));

//Echo(strcat("e,AU,",itoa(GetTick())));

return 0;

}
