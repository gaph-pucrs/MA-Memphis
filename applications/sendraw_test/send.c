#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>

int main()
{


    while(memphis_get_tick() < 220000);

    unsigned int message[21] = {
        
        //PROVAVELMENTE ALGO DE ERRADO AQUI

        0x00000000, //target (1x1)
        0x0000000B, //payload size (message header 11 + payload 8)
        0x00000001, //service (message delivery)
        257, //task_ID (app1 task1)
        256, //consumer = TARGET | MEMPHIS_KERNEL_MSG
        0x00000001, //source pe (1x1)
        memphis_get_tick(), //timestamp
        0x00000000, //transaction (empty)
        0, //msg_length (payload 32 bytes == 76 bytes)
        0x00000000, //???
        0x00000000, //???
        0x00000000, //???
        0x00000000, //???
        0x00000001, //payload
        0x00000002,
        0x00000003,
        0x00000004,
        0x00000005,
        0x00000006,
        0x00000007,
        0x00000008
    };

    printf("Sending message\n");

    // TASK_RELEASE
    // ID
    // TASK_CNT
    // *task_location
    memphis_send_raw(message, 13);
    printf("Message sent\n");

    while(1);

    return 0;
}
