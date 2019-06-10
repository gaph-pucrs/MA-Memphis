/*

                                (0)
                    (1)                     (2)
                (3)     (4)             (5)     (6)
              (7) (8) *(9)(10)        (11)(12) (13)(14)

    Iaçanã Ianiski Weber
    - iacanaw@gmail.com
    06/2019
*/
#include <api.h>
#include <stdlib.h>

#include "../quicksort_divider_conquer/sorting.h"

Message msg;

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
    int i, mensagens, tamanho;
    Echo("Aguardando o vetor!");
    Receive(&msg, sorting_4); // AQUI
    Echo("Recebendo o vetor:");
    Echo("--> Tamanho do vetor:");
    Echo(itoa(msg.length));
    tamanho = msg.length;
    Echo("Vetor:");
    for(i=0; i<tamanho; i++){
        vector[i] = msg.msg[i];
        Echo(itoa(vector[i]));
    }
    Echo("Começando QUICK SORT...");
    quickSort(vector, 0, EIGHT_VECTOR_SIZE-1);
    Echo("QUICK SORT finalizado!");

    Echo("Vetor organizado:");
    for(i=0; i<tamanho; i++){
        msg.msg[i] = vector[i];
        Echo(itoa(vector[i]));
    }
    msg.length = tamanho;
    Echo("Retornando o vetor:");
    Send(&msg, sorting_4); // AQUI

	exit();

}
