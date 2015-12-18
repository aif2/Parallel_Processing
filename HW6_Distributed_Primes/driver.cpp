#include <iostream>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <cstdlib>
#include "queue.h"
using namespace std;


int main(int argc, char *argv[])
{
    char c;
    int i;
    int shmid_range;
    int shmid_primes;

    int prime_min;
    int prime_max;

    // key of shared memory segment for shared queue of range
    key_t key_range = 1234;
    // key of shared memory segment for shared queue of primes
    key_t key_primes = 9999;

    char *shm, *s;

    // shared queue to store/read range of prime numbers by driver/processor
    struct queue *shared_queue_range;

    // shared queue to store/read all prime numbers by processor/driver
    struct queue *shared_queue_primes;    

    // check whether program arguments (range of prime numbers) have been passed
    if(argc < 3)
    {
        cout << endl;
        cout << "Usage: ./driver <MIN> <MAX> " << endl;
        cout << "where the arguments are as below:" << endl;
        cout << "MIN:   lower bound of range of prime numbers to be sent to processer program" << endl;
        cout << "MIN:   upper bound of range of prime numbers to be sent to processor program" << endl;
        cout << "e.g.   ./driver 0 1000 " << endl;
        cout << "above will instruct processor to find all prime numbers in the range 0-1000" << endl;
        cout << endl;
        exit(0);
    }

    prime_min = atoi(argv[1]);
    prime_max = atoi(argv[2]);

    /*
     * Create the shared memory segment for range
     */
    if ((shmid_range = shmget(key_range, sizeof(struct queue), IPC_CREAT | 0666)) < 0) {
        printf("error: %s", strerror(errno));
        perror("shmget() failed!\n");        
        exit(1);
    }
    
    /*
     * Now we attach the segment to our data space.
     */
    if ((shared_queue_range = (struct queue*)shmat(shmid_range, NULL, 0)) == (struct queue *) -1) {
        perror("shmat() failed!");
        exit(1);
    }

    /*
     * Create the shared memory segment for primes
     */
    if ((shmid_primes = shmget(key_primes, sizeof(struct queue), IPC_CREAT | 0666)) < 0) {
        printf("error: %s", strerror(errno));
        perror("shmget() failed!\n");        
        exit(1);
    }
    
    /*
     * Now we attach the segment to our data space.
     */
    if ((shared_queue_primes = (struct queue*)shmat(shmid_primes, NULL, 0)) == (struct queue *) -1) {
        perror("shmat() failed!");
        exit(1);
    }


    shared_queue_range->front = -1;
    shared_queue_range->end = -1;
    shared_queue_range->size = 0;

    shared_queue_primes->front = -1;
    shared_queue_primes->end = -1;
    shared_queue_primes->size = 0;


    enqueue(shared_queue_range, prime_min);
    enqueue(shared_queue_range, prime_max);

    while(true)
    {
        if(!isempty(shared_queue_primes))
        {
            int prime = dequeue(shared_queue_primes);
            if(prime > 0)
            {
                cout << prime << endl;    
            } 
            else if(prime == -2)
            {
                break;
            }            
        }     
    }

    // detach the shared queue of range
    shmdt(shared_queue_range);
    // detach the shared queue of primes
    shmdt(shared_queue_primes);

    // delete the shared memory segments
    shmctl(shmid_range, IPC_RMID, NULL);
    shmctl(shmid_primes, IPC_RMID, NULL);
    return 0;
}