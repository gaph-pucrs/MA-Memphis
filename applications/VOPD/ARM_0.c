#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>

int main()
{

int msg[128];
int j;

	for(j=0;j<128;j++) msg[j]=j;

	memphis_send(msg, sizeof(msg), IDCT2_0);
	
	return 0;

}
