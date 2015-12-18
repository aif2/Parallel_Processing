#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <cmath>
#include <cstdlib>
#include <pthread.h>          /* include the posix thread library */

#include "queue.h"

#define NUM_THREADS 4         /* number of threads to use */
#define PRIMES_COUNT 1000000  /* number of prime numbers to find */
#define SHMSZ     sizeof(queue<int>)*100

// data structure for each thread
struct thread_data{
    // lower bound of prime number 
    int low;
    // upper bound of prime number
    int high;
    // shared queue where thread will write the prime numbers
    struct queue *shared_queue;
};


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
 * find all prime numbers between range (low, high)
 */
void *thread_handler(void *arg)
{
    struct thread_data *ptr = (struct thread_data*)arg;
    int low = ptr->low;
    int high = ptr->high;
    struct queue *shared_queue = ptr->shared_queue;
    
    for(int n = low; n <= high; n++)
    {
        if(is_prime(n))
        {
            enqueue(shared_queue, n);
        }
    }
    
    pthread_exit(0);
}

using namespace std;

int main()
{
    // declare an array of 4 threads
    pthread_t  threads[NUM_THREADS];
    int start[NUM_THREADS];    
    int i = 2;    
    int shmid_range;
    int shmid_primes;

    // key of shared memory segment for shared queue of range
    key_t key_range = 1234;
    // key of shared memory segment for shared queue of primes
    key_t key_primes = 9999;

    char *shm, *s;

    // lower bound of range of prime numbers
    int prime_min = 0;
    // upper bound of range of prime numbers
    int prime_max = 0;


    // shared queue to store/read range of prime numbers by driver/processor
    struct queue *shared_queue_range;

    // shared queue to store/read all prime numbers by processor/driver
    struct queue *shared_queue_primes; 

    // lower bound of prime for a thread
    int low;
    // upper bound of prime for a thread
    int high;

    int rangePerThread;

    /*
     * Locate the segment for shared queue of range.
     */
    if ((shmid_range = shmget(key_range, sizeof(struct queue), 0666)) < 0) {
        perror("shmget() failed!");
        exit(1);
    }

    /*
     * Now we attach the segment to our data space.
     */
    if ((shared_queue_range = (struct queue *)shmat(shmid_range, NULL, 0)) == (struct queue*) -1) {
        perror("shmat() failed!");
        exit(1);
    }

    /*
     * Locate the segment for shared queue of range.
     */
    if ((shmid_primes = shmget(key_primes, sizeof(struct queue), 0666)) < 0) {
        perror("shmget() failed!");
        exit(1);
    }

    /*
     * Now we attach the segment to our data space.
     */
    if ((shared_queue_primes = (struct queue *)shmat(shmid_primes, NULL, 0)) == (struct queue*) -1) {
        perror("shmat() failed!");
        exit(1);
    }

    // read the bound of range from shared queue
    while(isempty(shared_queue_range))
    {
        sleep(1);    
    }

    prime_min = dequeue(shared_queue_range);
    prime_max = dequeue(shared_queue_range);

    //cout << prime_min << endl;
    //cout << prime_max << endl;

    shared_queue_primes->front = -1;
    shared_queue_primes->end = -1;
    shared_queue_primes->size = 0;


    // create NUM_THREADS threads and run them
    low = prime_min;
    rangePerThread = (prime_max - prime_min) / NUM_THREADS;
    high = low + rangePerThread - 1;
    i = 0;
    do {
        struct thread_data * thdata = new struct thread_data;
        thdata->low = low;
        thdata->high = high;
        thdata->shared_queue = shared_queue_primes;
        if(i == NUM_THREADS-1)
        {
            thdata->high = prime_max;
        }
        int status = pthread_create(&threads[i], NULL, thread_handler, (void *)thdata);
        if(status)
        {
            std::cout << "ERROR: pthread_create() failed!" << std::endl;
            exit(EXIT_FAILURE);
        }
        low = high + 1;
        high = low + rangePerThread - 1;
        i++;
    }while(i < NUM_THREADS);

    // wait for threads to complete
    for(int j = 0; j < NUM_THREADS; j++)
    {
        pthread_join(threads[j], NULL);
    }

    // put a end marker to queue
    enqueue(shared_queue_primes, -2);


     // detach the shared queue of range
    shmdt(shared_queue_range);
    // detach the shared queue of primes
    shmdt(shared_queue_primes);

    return 0;
}

