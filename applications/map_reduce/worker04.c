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

    int result_rcv;    memphis_receive(&result_rcv, sizeof(result_rcv), worker05);
        sum_cum += sum(&result_rcv, 1);
    memphis_receive(&result_rcv, sizeof(result_rcv), worker06);
        sum_cum += sum(&result_rcv, 1);

    int result = sum_cum;
    memphis_send(&result, sizeof(result), master);
    
    printf("Enviou\n");

    return 0;
}