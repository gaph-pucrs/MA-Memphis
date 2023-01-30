#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "map_reduce_std.h"


int main()
{
    int div_number = ARRAY_LEN/NUMBER_OF_TASKS;
    int *msg = (int*)malloc(div_number*sizeof(int));
    int *msg1 = (int*)malloc(div_number*sizeof(int));
    unsigned int sum_cum = 0;
    int values_to_sent = (ARRAY_LEN/NUMBER_OF_TASKS)*NUMBER_OF_TASKS;
    int missing_values = ARRAY_LEN-values_to_sent;
    int values_to_stay = missing_values+div_number;
    int *arr_malloc = (int*)malloc(values_to_stay*sizeof(int));

    printf("Fatiando e enviando...\n");

    for(int j=0; j<values_to_sent; j+=div_number) {
        if((j/div_number)+1 < NUMBER_OF_TASKS) {
            memcpy(msg, arr + j, div_number*sizeof(int));
            memphis_send(msg, div_number*sizeof(int), (j/div_number)+1);
        }
        else {
            memcpy(arr_malloc, arr + j, values_to_stay*sizeof(int));
        }
    }

    printf("Recebendo...\n");

    sum_cum = sum(arr_malloc, values_to_stay);

    memphis_receive(msg1, sizeof(msg1), worker01);
    sum_cum += sum(msg1, div_number);
    memphis_receive(msg1, sizeof(msg1), worker02);
    sum_cum += sum(msg1, div_number);
    memphis_receive(msg1, sizeof(msg1), worker04);
    sum_cum += sum(msg1, div_number);

    printf("Soma acumulada:  %d\n", sum_cum);

    return 0;
}