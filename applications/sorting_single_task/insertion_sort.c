/*
 * insertion_sort.c
 *
 *  Created on: 17 de set de 2018
 *      Author: mruaro
 */

#include <api.h>
#include <stdlib.h>
#include "../sorting_single_task/sorting_util.h"

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
void main()
{
    int n = sizeof(arr)/sizeof(arr[0]);
    Echo("n:"); Echo(itoa(n));
	Echo("Begin:");Echo(itoa(GetTick()));
	insertionSort(arr, n);
	Echo("End:");Echo(itoa(GetTick()));
	Echo("Sorted array");
	printArray(arr, n);
    exit();
}




