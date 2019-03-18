#include <api.h>
#include <stdlib.h>

int main()
{

Message msg;
int j;


//Echo(strcat("b,VU,",itoa(GetTick())));


	for(j=0;j<128;j++) msg.msg[j]=j;

	/*Comm SDRAM 3210*/
	msg.length=128;
	for(j=0;j<25;j++) Receive(&msg,SDRAM_0);
	msg.length=10;
	Receive(&msg,SDRAM_0);
	//Echo(strcat("r,MPEG_m19(3210),",itoa(GetTick())));
	//Echo(strcat(strcat(strcat("i,",itoa(i)),","),itoa(GetTick())));

//Echo(strcat("e,VU,",itoa(GetTick())));

exit();

}
