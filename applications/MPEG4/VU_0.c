#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>

int main()
{

int msg[128];
int j;


//Echo(strcat("b,VU,",itoa(GetTick())));


	for(j=0;j<128;j++) msg[j]=j;

	/*Comm SDRAM 3210*/
	for(j=0;j<25;j++) memphis_receive(msg, sizeof(msg), SDRAM_0);
	memphis_receive(msg, 10*sizeof(int), SDRAM_0);
	//Echo(strcat("r,MPEG_m19(3210),",itoa(GetTick())));
	//Echo(strcat(strcat(strcat("i,",itoa(i)),","),itoa(GetTick())));

//Echo(strcat("e,VU,",itoa(GetTick())));

return 0;

}
