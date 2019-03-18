#include <api.h>
#include <stdlib.h>

int main()
{

int j;
Message msg;

//Echo(strcat("b,RISC,",itoa(GetTick())));


	for(j=0;j<128;j++) msg.msg[j]=j;

	/*Comm SRAM2 8440*/
	msg.length=128;
	for(j=0;j<65;j++) Receive(&msg,SRAM2_0);
	msg.length=120;
	Receive(&msg,SRAM2_0);
	//Echo(strcat("r,MPEG_m(8440),",itoa(GetTick())));
	/*Comm SRAM2 8440*/
	msg.length=128;
	for(j=0;j<65;j++) Send(&msg,SRAM2_0);
	msg.length=120;
	Send(&msg,SRAM2_0);
	//Echo(strcat("s,MPEG_m2(8440),",itoa(GetTick())));
	//Echo(strcat(strcat(strcat("i,",itoa(i)),","),itoa(GetTick())));

//Echo(strcat("e,RISC,",itoa(GetTick())));

exit();

}
