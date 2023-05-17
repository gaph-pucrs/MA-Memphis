#include <memphis.h>
#include <stdio.h>

unsigned message[128];

int main()
{
	memphis_receive_raw(message, 128);

	int size = message[1] + 2;

	for(int i = 0; i < size; i++){
		printf("Flit %d = %x\n", i, message[i]);
	}

	return 0;
}
