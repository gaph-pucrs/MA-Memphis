#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>

int main()
{

int j;
int msg[128];

//Echo(strcat("b,IDCT,",itoa(GetTick())));


	for(j=0;j<128;j++) msg[j]=j;

	/*Comm SRAM2 4220*/
	for(j=0;j<32;j++) memphis_receive(msg, sizeof(msg), SRAM2_0);
	memphis_receive(msg, 124*sizeof(int), SRAM2_0);
	//Echo(strcat("r,MPEG_m4(4220),",itoa(GetTick())));
	/*Comm SRAM2 4220*/
	for(j=0;j<32;j++) memphis_send(msg, sizeof(msg), SRAM2_0);
	memphis_send(msg, 124*sizeof(int), SRAM2_0);
	//Echo(strcat("s,MPEG_m5(4220),",itoa(GetTick())));
	//Echo(strcat(strcat(strcat("i,",itoa(i)),","),itoa(GetTick())));

//Echo(strcat("e,IDCT,",itoa(GetTick())));

return 0;

}
