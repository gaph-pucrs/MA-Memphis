#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>

int main()
{

int j;
message_t msg;

//Echo(strcat("b,SDRAM,",itoa(GetTick())));


	for(j=0;j<128;j++) msg.payload[j]=j;

	/*Comm BAB 1280*/
	msg.length=128;
	for(j=0;j<10;j++) memphis_send(&msg,BAB_0);
	//Echo(strcat("s,MPEG_m6(1280),",itoa(GetTick())));
	/*Comm BAB 1280*/
	msg.length=128;
	for(j=0;j<10;j++) memphis_receive(&msg,BAB_0);
	//Echo(strcat("r,MPEG_m8(1280),",itoa(GetTick())));
	/*Comm UPSAMP 15000*/
	msg.length=128;
	for(j=0;j<117;j++)
	{
		//Echo("SEND upsamp antes\n");
		memphis_send(&msg,UPSAMP_0);
		//Echo("SEND upsamp depois\n");
	}
	msg.length=24;
	memphis_send(&msg,UPSAMP_0);
	//Echo(strcat("s,MPEG_m9(15000),",itoa(GetTick())));
	/*Comm UPSAMP 15000*/
	msg.length=128;
	for(j=0;j<117;j++)
	{
		//Echo("receive upsamp antes\n");
		memphis_receive(&msg,UPSAMP_0);
		//Echo("receive upsamp depois\n");
	}
	msg.length=24;
	memphis_receive(&msg,UPSAMP_0);
	//Echo(strcat("r,MPEG_m10(15000),",itoa(GetTick())));
	/*Comm ADSP 1280*/
	msg.length=128;
	for(j=0;j<10;j++) memphis_send(&msg,ADSP_0);
	//Echo(strcat("s,MPEG_m11(1280),",itoa(GetTick())));
	/*Comm MCPU 1280*/
	msg.length=128;
	for(j=0;j<10;j++) memphis_send(&msg,MCPU_0);
	//Echo(strcat("s,MPEG_m12(1280),",itoa(GetTick())));
	/*Comm ADSP 1280*/
	msg.length=128;
	for(j=0;j<10;j++) memphis_receive(&msg,ADSP_0);
	//Echo(strcat("r,MPEG_m13(1280),",itoa(GetTick())));
	/*Comm RAST 10130*/
	msg.length=128;
	for(j=0;j<79;j++) memphis_send(&msg,RAST_0);
	msg.length=18;
	memphis_send(&msg,RAST_0);
	//Echo(strcat("s,MPEG_m16(10130),",itoa(GetTick())));
	/*Comm RAST 10130*/
	msg.length=128;
	for(j=0;j<79;j++) memphis_receive(&msg,RAST_0);
	msg.length=18;
	memphis_receive(&msg,RAST_0);
	//Echo(strcat("r,MPEG_m17(10130),",itoa(GetTick())));
	/*Comm AU 1280*/
	msg.length=128;
	for(j=0;j<10;j++) memphis_send(&msg,AU_0);
	//Echo(strcat("s,MPEG_m18(1280),",itoa(GetTick())));
	/*Comm VU 3210*/
	msg.length=128;
	for(j=0;j<25;j++) memphis_send(&msg,VU_0);
	msg.length=10;
	memphis_send(&msg,VU_0);
	//Echo(strcat("s,MPEG_m19(3210),",itoa(GetTick())));
	//Echo(strcat(strcat(strcat("i,",itoa(i)),","),itoa(GetTick())));

//Echo(strcat("e,SDRAM,",itoa(GetTick())));

return 0;

}
