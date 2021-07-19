/*
 * selection_sorting.c
 *
 *  Created on: 17 de set de 2018
 *      Author: mruaro
 */

#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>
#include "sorting_util.h"

// C program for implementation of selection sort
void selectionSort(int arr[], int n)
{
    int i, j, min_idx;

    // One by one move boundary of unsorted subarray
    for (i = 0; i < n-1; i++)
    {
        // Find the minimum element in unsorted array
        min_idx = i;
        for (j = i+1; j < n; j++)
          if (arr[j] < arr[min_idx])
            min_idx = j;

        // Swap the found minimum element with the first element
        swap(&arr[min_idx], &arr[i]);
    }
}


// Driver program to test above functions
int main()
{
    int n = sizeof(arr)/sizeof(arr[0]);
    printf("n: %d\n",n); 
   	printf("Begin: %d\n", memphis_get_tick());
	selectionSort(arr, n);
	printf("End: %d\n", memphis_get_tick());
   	puts("Sorted array\n");
	printArray(arr, n);
	return 0;
}


