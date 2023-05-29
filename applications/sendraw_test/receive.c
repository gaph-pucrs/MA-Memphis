#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>

#define MSG_LEN 8 //words
#define MSG_SIZE (MSG_LEN << 2) //bytes

int main()
{
    unsigned int payload[MSG_LEN];
    while(1);

    // printf("Receiving message\n");

    // memphis_receive(payload, MSG_SIZE, send);
    // printf("Message received\n");

    // int i = 0;
    // while(i < MSG_LEN){
    //     printf("%u\n", payload[i]);
    //     i++;
    // }

    return 0;
}
