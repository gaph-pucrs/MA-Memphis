#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>

int main()
{

message_t msg;
int j;


//Echo(strcat("b,VU,",itoa(GetTick())));


	for(j=0;j<128;j++) msg.payload[j]=j;

	/*Comm SDRAM 3210*/
	msg.length=128;
	for(j=0;j<25;j++) memphis_receive(&msg,SDRAM_0);
	msg.length=10;
	memphis_receive(&msg,SDRAM_0);
	//Echo(strcat("r,MPEG_m19(3210),",itoa(GetTick())));
	//Echo(strcat(strcat(strcat("i,",itoa(i)),","),itoa(GetTick())));

//Echo(strcat("e,VU,",itoa(GetTick())));

return 0;

}
