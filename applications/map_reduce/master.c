#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>
#include "map_reduce_std.h"

message_t msg;
message_t msg1;

int main()
{
    int div_number = ARRAY_LEN/NUMBER_OF_TASKS;
    unsigned int sum_cum = 0;
    int values_to_sent = (ARRAY_LEN/NUMBER_OF_TASKS)*NUMBER_OF_TASKS;
    int missing_values = ARRAY_LEN-values_to_sent;
    int values_to_stay = missing_values+div_number;
    unsigned int arr_malloc[values_to_stay];

    printf("Fatiando e enviando...\n");

    for(int j=0; j<values_to_sent; j+=div_number) {
        if((j/div_number)+1 < NUMBER_OF_TASKS) {
            __builtin_memcpy(msg.payload, arr + j, div_number*sizeof(int));
            msg.length = div_number;
            memphis_send(&msg, (j/div_number)+1);
        }
        else {
            __builtin_memcpy(arr_malloc, arr + j, values_to_stay*sizeof(int));
        }
    }

    printf("Recebendo...\n");

    sum_cum = sum(arr_malloc, values_to_stay);

    memphis_receive(&msg1, worker01);
    sum_cum += sum(msg1.payload, div_number);
    memphis_receive(&msg1, worker02);
    sum_cum += sum(msg1.payload, div_number);
    memphis_receive(&msg1, worker04);
    sum_cum += sum(msg1.payload, div_number);
    memphis_receive(&msg1, worker08);
    sum_cum += sum(msg1.payload, div_number);
    memphis_receive(&msg1, worker16);
    sum_cum += sum(msg1.payload, div_number);

    printf("Soma acumulada:  %d\n", sum_cum);

    return 0;
}