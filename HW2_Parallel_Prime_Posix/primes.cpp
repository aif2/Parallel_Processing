//Anhar Felimban
//Intro to parallel processing
//Fall 2015

#include <iostream>
#include <cmath>
#include <cstdlib>
#include <pthread.h>          /* include the posix thread library */

#define NUM_THREADS 4         /* number of threads to use */
#define PRIMES_COUNT 1000000  /* number of prime numbers to find */


// number of primes to find per thread
int primesPerThread = PRIMES_COUNT / NUM_THREADS;


bool is_prime(int n)
{
    
    auto j = static_cast<int>(sqrt(n));
    
    for (int i = 2; i <= j; i++)
    {
        if (n % i == 0) return false;
    }
    return true;
}

/*
 * This function is executed by each thread to
 * find all "primesPerThread" number of prime numbers with passed
 * argument integer as the starting value.
 *
 * Input:  arg a pointer to an integer value
 * Output: print all prime numbers P where *arg <= p <= (*arg + primesPerThread)
 */
void *thread_handler(void *arg)
{
    int *ptr = (int *)arg;
    int start = *ptr;
    
    for(int n = start; n < (start + 250000); n++)
    {
        if(is_prime(n))
        {
            printf("%d\n", n);
        }
    }
    
    pthread_exit(0);
}


int main()
{
    // declare an array of 4 threads
    pthread_t  threads[NUM_THREADS];
    int start[NUM_THREADS];
    
    int i = 2;
    
    // loop to distribute the prime numbers among threads
    for(int j=0; j < NUM_THREADS; j++)
    {
        start[j] = i;
        //std::cout << *start << std::endl;
        int status = pthread_create(&threads[j], NULL, thread_handler, (void *)&start[j]);
        if(status)
        {
            std::cout << "ERROR: pthread_create() failed!" << std::endl;
            exit(EXIT_FAILURE);
        }
        //i += primesPerThread;
        i += 250000;
    }
    
    
    // wait for all threads to terminate
    for(int j = 0; j < NUM_THREADS; j++)
    {
        pthread_join(threads[j], NULL);
    }
    
    exit(EXIT_SUCCESS);
}

