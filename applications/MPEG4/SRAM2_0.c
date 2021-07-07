#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>

int main()
{

int j;
message_t msg;

//Echo(strcat("b,SRAM2,",itoa(GetTick())));


	for(j=0;j<128;j++) msg.payload[j]=j;

	/*Comm RISC 8440*/
	msg.length=128;
	for(j=0;j<65;j++) memphis_send(&msg,RISC_0);
	msg.length=120;
	memphis_send(&msg,RISC_0);
	//Echo(strcat("s,MPEG_m(8440),",itoa(GetTick())));
	/*Comm RISC 8440*/
	msg.length=128;
	for(j=0;j<65;j++) memphis_receive(&msg,RISC_0);
	msg.length=120;
	memphis_receive(&msg,RISC_0);
	//Echo(strcat("r,MPEG_m2(8440),",itoa(GetTick())));
	/*Comm BAB 2930*/
	msg.length=128;
	for(j=0;j<22;j++) memphis_send(&msg,BAB_0);
	msg.length=114;
	memphis_send(&msg,BAB_0);
	//Echo(strcat("s,MPEG_m3(2930),",itoa(GetTick())));
	/*Comm IDCT 4220*/
	msg.length=128;
	for(j=0;j<32;j++) memphis_send(&msg,IDCT_0);
	msg.length=124;
	memphis_send(&msg,IDCT_0);
	//Echo(strcat("s,MPEG_m4(4220),",itoa(GetTick())));
	/*Comm IDCT 4220*/
	msg.length=128;
	for(j=0;j<32;j++) memphis_receive(&msg,IDCT_0);
	msg.length=124;
	memphis_receive(&msg,IDCT_0);
	//Echo(strcat("r,MPEG_m5(4220),",itoa(GetTick())));
	/*Comm UPSAMP 11310*/
	msg.length=128;
	for(j=0;j<88;j++) memphis_send(&msg,UPSAMP_0);
	msg.length=46;
	memphis_send(&msg,UPSAMP_0);
	//Echo(strcat("s,MPEG_m7(11310),",itoa(GetTick())));
	//Echo(strcat(strcat(strcat("i,",itoa(i)),","),itoa(GetTick())));

//Echo(strcat("e,SRAM2,",itoa(GetTick())));

return 0;

}
