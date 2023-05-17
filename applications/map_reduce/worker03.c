#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "map_reduce_std.h"

#define MAX_MESSAGE_SIZE 128
int msg[256];

int main()
{
    unsigned int sum_cum = 0;

    printf("Worker%d\n", getpid()-255-1);

    unsigned received_bytes = memphis_receive(msg, sizeof(msg), master);

    printf("Recebeu\n");
	
    sum_cum += sum(msg, received_bytes/sizeof(int));

    int result =sum_cum;
    memphis_send(&result, sizeof(result), worker02);
    
    printf("Enviou\n");

    return 0;
}