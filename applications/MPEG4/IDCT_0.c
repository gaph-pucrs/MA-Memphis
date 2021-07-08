#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>

int main()
{

int j;
message_t msg;

//Echo(strcat("b,IDCT,",itoa(GetTick())));


	for(j=0;j<128;j++) msg.payload[j]=j;

	/*Comm SRAM2 4220*/
	msg.length=128;
	for(j=0;j<32;j++) memphis_receive(&msg,SRAM2_0);
	msg.length=124;
	memphis_receive(&msg,SRAM2_0);
	//Echo(strcat("r,MPEG_m4(4220),",itoa(GetTick())));
	/*Comm SRAM2 4220*/
	msg.length=128;
	for(j=0;j<32;j++) memphis_send(&msg,SRAM2_0);
	msg.length=124;
	memphis_send(&msg,SRAM2_0);
	//Echo(strcat("s,MPEG_m5(4220),",itoa(GetTick())));
	//Echo(strcat(strcat(strcat("i,",itoa(i)),","),itoa(GetTick())));

//Echo(strcat("e,IDCT,",itoa(GetTick())));

return 0;

}
