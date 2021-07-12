/*

                                (0)
                    (1)                     (2)
                (3)     (4)             (5)     (6)
              (7) *(8) (9)(10)        (11)(12) (13)(14)

    Iaçanã Ianiski Weber
    - iacanaw@gmail.com
    06/2019
*/
#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>

#include "../quicksort_divider_conquer/sorting.h"

message_t msg;

void swap(int *xp, int *yp)
{
    int temp = *xp;
    *xp = *yp;
    *yp = temp;
}

/* This function takes last element as pivot, places
   the pivot element at its correct position in sorted
    array, and places all smaller (smaller than pivot)
   to left of pivot and all greater elements to right
   of pivot */
int partition (int *arr, int low, int high)
{
    int pivot = arr[high];    // pivot
    int i = (low - 1);  // Index of smaller element

    for (int j = low; j <= high- 1; j++)
    {
        // If current element is smaller than or
        // equal to pivot
        if (arr[j] <= pivot)
        {
            i++;    // increment index of smaller element
            swap(&arr[i], &arr[j]);
        }
    }
    swap(&arr[i + 1], &arr[high]);
    return (i + 1);
}

/* The main function that implements QuickSort
 arr[] --> Array to be sorted,
  low  --> Starting index,
  high  --> Ending index */
void quickSort(int *arr, int low, int high)
{
    if (low < high)
    {
        /* pi is partitioning index, arr[p] is now
           at right place */
        int pi = partition(arr, low, high);

        // Separately sort elements before
        // partition and after partition
        quickSort(arr, low, pi - 1);
        quickSort(arr, pi + 1, high);
    }
}

int main(){
    int vector[EIGHT_VECTOR_SIZE];
    int i, tamanho;
    puts("Aguardando o vetor!\n");
    memphis_receive(&msg, sorting_3); // AQUI
    puts("Recebendo o vetor:\n");
    puts("--> Tamanho do vetor:\n");
    printf("%d\n", msg.length);
    tamanho = msg.length;
    puts("Vetor:\n");
    for(i=0; i<tamanho; i++){
        vector[i] = msg.payload[i];
        printf("%d\n", vector[i]);
    }
    puts("Começando QUICK SORT...\n");
    quickSort(vector, 0, EIGHT_VECTOR_SIZE-1);
    puts("QUICK SORT finalizado!\n");

    puts("Vetor organizado:\n");
    for(i=0; i<tamanho; i++){
        msg.payload[i] = vector[i];
        printf("%d\n", vector[i]);
    }
    msg.length = tamanho;
    puts("Retornando o vetor:\n");
    memphis_send(&msg, sorting_3); // AQUI

	return 0;

}
