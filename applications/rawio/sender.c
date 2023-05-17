#include <memphis.h>
#include <stdio.h>

unsigned message[128];

int main()
{
	while(memphis_get_tick() < 500000);
	printf("Sending RAW\n");
	message[0] = 0x0101;			/* Target */
	message[1] = 11;				/* Size disregarding target and size flits */
	message[2] = ABORT_TASK;		/* Service */
	memphis_send_raw(message, 13);	/* Minimum of 13 flits */
	printf("Sent RAW\n");

	return 0;
}
