#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>

int main()
{
		
	unsigned int message[21] = {
		
		0x00000202,	//target (2x2)
		0x00000013,	//payload size (message header 11 + message payload 8)
		0x00000001,	//service (task delivery)
		257,		//task_ID (app1 task1) -- 257
		256,		//consumer -- 256
		0x00000101,	//source pe (1x1)
		memphis_get_tick(),	//timestamp
		0x00000000,
		32,			//msg_length (payload 32 bytes)
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000001,	//payload
		0x00000002,
		0x00000003,
		0x00000004,
		0x00000005,
		0x00000006,
		0x00000007,
		0x00000008
	};

	printf("Sending message\n");
	memphis_send_raw(message, 21);
	printf("Message sent\n");

	message[13] = 9;
	message[14] = 10;
	message[15] = 11;
	message[16] = 12;
	message[17] = 13;
	message[18] = 14;
	message[19] = 15;
	message[20] = 16;

	while(memphis_get_tick() < 700000);

	printf("Sending message\n");
	memphis_send_raw(message, 21);
	printf("Message sent\n");

	while(1);

	return 0;
}
