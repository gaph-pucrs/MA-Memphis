#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>

int main()
{

int j;
int msg[128];

//Echo(strcat("b,SRAM2,",itoa(GetTick())));


	for(j=0;j<128;j++) msg[j]=j;

	/*Comm RISC 8440*/
	for(j=0;j<65;j++) memphis_send(msg, sizeof(msg), RISC_0);
	memphis_send(msg, 120*sizeof(int), RISC_0);
	//Echo(strcat("s,MPEG_m(8440),",itoa(GetTick())));
	/*Comm RISC 8440*/
	for(j=0;j<65;j++) memphis_receive(msg, sizeof(msg), RISC_0);
	memphis_receive(msg, 120*sizeof(int), RISC_0);
	//Echo(strcat("r,MPEG_m2(8440),",itoa(GetTick())));
	/*Comm BAB 2930*/
	for(j=0;j<22;j++) memphis_send(msg, sizeof(msg), BAB_0);
	memphis_send(msg, 114*sizeof(int), BAB_0);
	//Echo(strcat("s,MPEG_m3(2930),",itoa(GetTick())));
	/*Comm IDCT 4220*/
	for(j=0;j<32;j++) memphis_send(msg, sizeof(msg), IDCT_0);
	memphis_send(msg, 124*sizeof(int), IDCT_0);
	//Echo(strcat("s,MPEG_m4(4220),",itoa(GetTick())));
	/*Comm IDCT 4220*/
	for(j=0;j<32;j++) memphis_receive(msg, sizeof(msg), IDCT_0);
	memphis_receive(msg, 124*sizeof(int), IDCT_0);
	//Echo(strcat("r,MPEG_m5(4220),",itoa(GetTick())));
	/*Comm UPSAMP 11310*/
	for(j=0;j<88;j++) memphis_send(msg, sizeof(msg), UPSAMP_0);
	memphis_send(msg, 46*sizeof(int), UPSAMP_0);
	//Echo(strcat("s,MPEG_m7(11310),",itoa(GetTick())));
	//Echo(strcat(strcat(strcat("i,",itoa(i)),","),itoa(GetTick())));

//Echo(strcat("e,SRAM2,",itoa(GetTick())));

return 0;

}
