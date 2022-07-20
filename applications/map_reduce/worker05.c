#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "map_reduce_std.h"

message_t msg;
message_t msg1;

int main()
{
    unsigned int sum_cum = 0;

    printf("Worker%d\n", getpid()-255-1);

    memphis_receive(&msg, master);

    printf("Recebeu\n");
	
    sum_cum += sum(msg.payload, msg.length);

    msg1.payload[0]=sum_cum;
    msg1.length = 1;
    memphis_send(&msg1, worker04);
    
    printf("Enviou\n");

    return 0;
}