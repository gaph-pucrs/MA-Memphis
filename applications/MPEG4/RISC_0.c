#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>

int main()
{

int j;
message_t msg;

//Echo(strcat("b,RISC,",itoa(GetTick())));


	for(j=0;j<128;j++) msg.payload[j]=j;

	/*Comm SRAM2 8440*/
	msg.length=128;
	for(j=0;j<65;j++) memphis_receive(&msg,SRAM2_0);
	msg.length=120;
	memphis_receive(&msg,SRAM2_0);
	//Echo(strcat("r,MPEG_m(8440),",itoa(GetTick())));
	/*Comm SRAM2 8440*/
	msg.length=128;
	for(j=0;j<65;j++) memphis_send(&msg,SRAM2_0);
	msg.length=120;
	memphis_send(&msg,SRAM2_0);
	//Echo(strcat("s,MPEG_m2(8440),",itoa(GetTick())));
	//Echo(strcat(strcat(strcat("i,",itoa(i)),","),itoa(GetTick())));

//Echo(strcat("e,RISC,",itoa(GetTick())));

return 0;

}
