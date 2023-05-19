#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>

int main()
{

int j;
int msg[128];

//Echo(strcat("b,SDRAM,",itoa(GetTick())));


	for(j=0;j<128;j++) msg[j]=j;

	/*Comm BAB 1280*/
	for(j=0;j<10;j++) memphis_send(msg, sizeof(msg), BAB_0);
	//Echo(strcat("s,MPEG_m6(1280),",itoa(GetTick())));
	/*Comm BAB 1280*/
	for(j=0;j<10;j++) memphis_receive(msg, sizeof(msg), BAB_0);
	//Echo(strcat("r,MPEG_m8(1280),",itoa(GetTick())));
	/*Comm UPSAMP 15000*/
	for(j=0;j<117;j++)
	{
		//Echo("SEND upsamp antes\n");
		memphis_send(msg, sizeof(msg), UPSAMP_0);
		//Echo("SEND upsamp depois\n");
	}
	memphis_send(msg, 24*sizeof(int), UPSAMP_0);
	//Echo(strcat("s,MPEG_m9(15000),",itoa(GetTick())));
	/*Comm UPSAMP 15000*/
	for(j=0;j<117;j++)
	{
		//Echo("receive upsamp antes\n");
		memphis_receive(msg, sizeof(msg), UPSAMP_0);
		//Echo("receive upsamp depois\n");
	}
	memphis_receive(msg, 24*sizeof(int), UPSAMP_0);
	//Echo(strcat("r,MPEG_m10(15000),",itoa(GetTick())));
	/*Comm ADSP 1280*/
	for(j=0;j<10;j++) memphis_send(msg, sizeof(msg), ADSP_0);
	//Echo(strcat("s,MPEG_m11(1280),",itoa(GetTick())));
	/*Comm MCPU 1280*/
	for(j=0;j<10;j++) memphis_send(msg, sizeof(msg), MCPU_0);
	//Echo(strcat("s,MPEG_m12(1280),",itoa(GetTick())));
	/*Comm ADSP 1280*/
	for(j=0;j<10;j++) memphis_receive(msg, sizeof(msg), ADSP_0);
	//Echo(strcat("r,MPEG_m13(1280),",itoa(GetTick())));
	/*Comm RAST 10130*/
	for(j=0;j<79;j++) memphis_send(msg, sizeof(msg), RAST_0);
	memphis_send(msg, 18*sizeof(int), RAST_0);
	//Echo(strcat("s,MPEG_m16(10130),",itoa(GetTick())));
	/*Comm RAST 10130*/
	for(j=0;j<79;j++) memphis_receive(msg, sizeof(msg), RAST_0);
	memphis_receive(msg, 18*sizeof(int), RAST_0);
	//Echo(strcat("r,MPEG_m17(10130),",itoa(GetTick())));
	/*Comm AU 1280*/
	for(j=0;j<10;j++) memphis_send(msg, sizeof(msg), AU_0);
	//Echo(strcat("s,MPEG_m18(1280),",itoa(GetTick())));
	/*Comm VU 3210*/
	for(j=0;j<25;j++) memphis_send(msg, sizeof(msg), VU_0);
	memphis_send(msg, 10*sizeof(int), VU_0);
	//Echo(strcat("s,MPEG_m19(3210),",itoa(GetTick())));
	//Echo(strcat(strcat(strcat("i,",itoa(i)),","),itoa(GetTick())));

//Echo(strcat("e,SDRAM,",itoa(GetTick())));

return 0;

}
