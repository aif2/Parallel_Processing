//Anhar Felimban
//Intro to parallel processing
//Fall 2015

#include <iostream>
#include <cmath>
#include <cstdlib>
#include <pthread.h>          /* include the posix thread library */

#define MAX_SIZE 1000000      /* maximum size of array to sort */ 

using namespace std;

/* structure to store details of partial array sorted by a thread */
struct array_interval    
{
	int *arr;
	int start;
	int end;
};

/*
 * This function is executed by each thread to 
 * sort a partial array indicated by start and end
 * index positions in the array.
 */
void *thread_sort(void *arg);

/**
 * Mergesort an array from a given start and end
 * position indices and return a copy of the sorted
 * array.
 */
int *mergesort(int *arr, int start, int end);

/**
 * Merge two arrays with given size to produce a 
 * sorted array and return the sorted array
 */
 int *merge(int* arr1, int *arr2, int n1, int n2);

/**
 * Generate an array of size n with
 * random integers in the range 0 - MAX_SIZE
 */
int *generate_array(int n);

bool isarray_sorted(int *arr, int n);

int main()
{
	// array of integers to sort
	int *arr;

	// array of threads
	pthread_t  *threads;

	// array of partial array sort details structure
	struct array_interval *arrint;

	// number of threads to execute
	int numThreads;

	// size of partial array sorted by each thread
	int arraySizePerThread;

	int i; 

	// generate a random array
	time_t t;
	srand(time(&t));
	arr = generate_array(MAX_SIZE);

	// read number of threads from user
	cout << "Enter number of threads: ";
	cin >> numThreads;

	arraySizePerThread = MAX_SIZE / numThreads;

	// allocate space for threads
	threads = new pthread_t[numThreads];
	// allocate space for partial array sort structures
	arrint = new struct array_interval[numThreads];
	
	int start = 0;
	int end = start + arraySizePerThread - 1;

	// loop to create and run the threads
	for(int j=0; j < numThreads; j++)
	{
		// prepare the next partial array sort structure
		arrint[j].start = start;
		arrint[j].end = end;
		arrint[j].arr = arr;

		// create thread and run it 
		int status = pthread_create(&threads[j], NULL, thread_sort, (void *)&arrint[j]);
		if(status)
		{
			std::cout << "ERROR: pthread_create() failed!" << std::endl;
			exit(EXIT_FAILURE);
		}
		// update next start and end position
		start += arraySizePerThread;
		end = start + arraySizePerThread - 1;
	}
	
	// wait for all threads to terminate
	for(int j = 0; j < numThreads; j++)
	{
		pthread_join(threads[j], NULL);
	}
		
	// sequentially merge all the array segments sorted by threads
	
	int *arr1 = new int[arraySizePerThread];
	int *arr2 = new int[arraySizePerThread];
	int k=0;
	for(i = 0; i < arraySizePerThread; i++) arr1[i] = arr[i];
	for(i = arraySizePerThread; i < 2*arraySizePerThread; i++, k++) arr2[k] = arr[i];
	arr1 = merge(arr1, arr2, arraySizePerThread, arraySizePerThread);
	int n1 = arraySizePerThread*2;
	int n2;
	for(i = 2; i < numThreads; i++)
	{
		int j, k=0;
		int start = arrint[i].start;
		int end = arrint[i].end;
		arr2 = new int[end - start + 1];
		for(j = start; j <= end; j++,k++) arr2[k] = arr[j];
		n2 = (end - start + 1);
	    arr1 = merge(arr1, arr2, n1, n2);
	    n1 += n2;
	}
	for(i = 0; i < MAX_SIZE; i++) arr[i] = arr1[i];
	

	// display the sorted array
	for(i=0; i < MAX_SIZE; i++)
	{
		cout << arr[i] << endl;
	}

	// check whether array has been sorted
	if(isarray_sorted(arr, MAX_SIZE))
		cout << endl << "thread sorted: yes" << endl;
	else
		cout << endl << "thread sorted: no" << endl;

    exit(EXIT_SUCCESS);
}



void *thread_sort(void *arg)
{
	struct array_interval *arrint = (struct array_interval *)arg;
	int start = arrint->start;
	int end = arrint->end;
	int *arr = arrint->arr;
	int i,k=0;
	int *sortedarr = mergesort(arr, start, end);
	for(i = start; i <= end; i++,k++)
	{
		//arr[i] = sortedarr[k];
	}

	pthread_exit(0);
}

int *mergesort(int *arr, int start, int end)
{
	int *sortedarr;
	int i;
	if(start == end)
	{
		sortedarr = new int[1];
		sortedarr[0] = arr[start];
	}
	else
	{
		int mid = (start + end) / 2;
		int *sortedarrleft = mergesort(arr, start, mid);
		int *sortedarrright = mergesort(arr, mid+1, end);
		int n1 = (mid - start + 1);
		int n2 = (end - mid);
		sortedarr = merge(sortedarrleft, sortedarrright, n1, n2);
		for(i = 0; i < (end - start + 1); i++)
		{
			arr[i+start] = sortedarr[i];
		}
		delete sortedarrleft;
		delete sortedarrright;
	}

	return sortedarr;
}

int *merge(int* arr1, int *arr2, int n1, int n2)
 {
 	int *sortedarr = new int[n1 + n2];
 	int i=0, j=0, k=0;
 	while ( i < n1 && j < n2)
 	{
 		if(arr1[i] < arr2[j])
 		{
 			sortedarr[k++] = arr1[i];
 			i++;
 		}
 		else
 		{
 			sortedarr[k++] = arr2[j];
 			j++;
 		}
 	}
 	while( i < n1)
 	{
 		sortedarr[k++] = arr1[i];
 		i++;
 	}
 	while( j < n2)
 	{
 		sortedarr[k++] = arr2[j];
 		j++;
 	}
 	return sortedarr;
 }

int *generate_array(int n)
{
	int *arr = new int[n];
	int i;
	for(i=0; i < n; i++)
	{
		arr[i] = rand() % MAX_SIZE;
	}
	return arr;
}

bool isarray_sorted(int *arr, int n)
{
	int i;
	for(i = 0; i < n-1; i++)
	{
		if(arr[i] > arr[i+1])
			return false;
	}
	return true;
}