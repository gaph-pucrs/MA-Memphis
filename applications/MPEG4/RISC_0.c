#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>

int main()
{

int j;
int msg[128];

//Echo(strcat("b,RISC,",itoa(GetTick())));


	for(j=0;j<128;j++) msg[j]=j;

	/*Comm SRAM2 8440*/
	for(j=0;j<65;j++) memphis_receive(msg, sizeof(msg), SRAM2_0);
	memphis_receive(msg, 120*sizeof(int), SRAM2_0);
	//Echo(strcat("r,MPEG_m(8440),",itoa(GetTick())));
	/*Comm SRAM2 8440*/
	for(j=0;j<65;j++) memphis_send(msg, sizeof(msg), SRAM2_0);
	memphis_send(msg, 120*sizeof(int), SRAM2_0);
	//Echo(strcat("s,MPEG_m2(8440),",itoa(GetTick())));
	//Echo(strcat(strcat(strcat("i,",itoa(i)),","),itoa(GetTick())));

//Echo(strcat("e,RISC,",itoa(GetTick())));

return 0;

}
