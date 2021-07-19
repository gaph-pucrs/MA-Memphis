/*
 * insertion_sort.c
 *
 *  Created on: 17 de set de 2018
 *      Author: mruaro
 */

#include <memphis.h>
#include <stdlib.h>
#include <stdio.h>
#include "sorting_util.h"

/* Function to sort an array using insertion sort*/
void insertionSort(int arr[], int n)
{
   int i, key, j;
   for (i = 1; i < n; i++)
   {
       key = arr[i];
       j = i-1;

       /* Move elements of arr[0..i-1], that are
          greater than key, to one position ahead
          of their current position */
       while (j >= 0 && arr[j] > key)
       {
           arr[j+1] = arr[j];
           j = j-1;
       }
       arr[j+1] = key;
   }
}


/* Driver program to test insertion sort */
int main()
{
    int n = sizeof(arr)/sizeof(arr[0]);
    printf("n: %d\n",n); 
   	printf("Begin: %d\n", memphis_get_tick());
	insertionSort(arr, n);
	printf("End: %d\n", memphis_get_tick());
   	puts("Sorted array\n");
	printArray(arr, n);
    return 0;
}




