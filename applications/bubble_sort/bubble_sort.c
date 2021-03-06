/*
 * bublle_sorting.c
 *
 *  Created on: 17 de set de 2018
 *      Author: mruaro
 */

#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>
#include "sorting_util.h"

// A function to implement bubble sort
void bubbleSort(int arr[], int n)
{
   int i, j;
   for (i = 0; i < n-1; i++)

       // Last i elements are already in place
       for (j = 0; j < n-i-1; j++)
           if (arr[j] > arr[j+1])
              swap(&arr[j], &arr[j+1]);
}

// Driver program to test above functions
int main()
{
    int n = sizeof(arr)/sizeof(arr[0]);
    printf("n: %d\n",n); 
   	printf("Begin: %d\n", memphis_get_tick());
   	bubbleSort(arr, n);
   	printf("End: %d\n", memphis_get_tick());
   	puts("Sorted array\n");
   	printArray(arr, n);
    return 0;
}
