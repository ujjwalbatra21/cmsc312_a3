/*
 * server.c
 *
 *  Created on: Apr 20, 2021
 *      Author: chrispy
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <pthread.h>
 #include <fcntl.h>           /* For O_* constants */
 #include <sys/stat.h>        /* For mode constants */
 #include <semaphore.h>
 #include <sys/shm.h>

 #include "Csem_correct.h"

 #define SIZE 5
 #define NUMB_THREADS 6
 #define PRODUCER_LOOPS 2


 int buffer[SIZE];
 int buffer_index;

// pthread_mutex_t buffer_mutex;
 /* initially buffer will be empty.  full_sem
    will be initialized to buffer SIZE, which means
    SIZE number of producer threads can write to it.
    And empty_sem will be initialized to 0, so no
    consumer can read from buffer until a producer
    thread posts to empty_sem */
 // struct Csem_t full_sem;  /* when 0, buffer is full */
 // struct Csem_t empty_sem; /* when 0, buffer is empty. Kind of
 //                    like an index for the buffer */

int *buff_shm, *buffind_shm;

 // sem_t *full_sem;
 // sem_t *empty_sem;
 // sem_t *mutex_sem;
 // char *name_full = "/full_sem";
 // char *name_empty = "/empty_sem";
 // char *name_mutex = "/mutex_sem";
// Csem_t full_sem;
// Csem_t empty_sem;
Csem_t *empty_shm, *full_shm, *mutex_shm;
 // Csem_t full_sem_p;
 // Csem_t empty_sem_p;


 void insertbuffer(int value) {
     if (*buffind_shm < SIZE) {
         *(buff_shm + (*buffind_shm++)) = value;
     } else {
         printf("Buffer overflow\n");
     }
 }

 int dequeuebuffer() {
     if (*buffind_shm > 0) {
         return *(buff_shm + (*buffind_shm--)); // buffer_index-- would be error!
     } else {
         printf("Buffer underflow\n");
     }
     return 0;
 }


 void *consumer(void *thread_n) {
     int thread_numb = *(int *)thread_n;
     int value;
     int i=0;
	 printf("yeet\n");
     while (i++ < PRODUCER_LOOPS) {
		 printf("enter loop\n");
         Csem_wait(empty_shm);
         /* there could be race condition here, that could cause
            buffer underflow error */
		 printf("wait_empty\n");
         Csem_wait(mutex_shm);
		 printf("wait_mutex\n");
         value = dequeuebuffer(value);
         Csem_post(mutex_shm);
		 printf("post_mutex\n");
         Csem_post(full_shm); // post (increment) fullbuffer semaphore
		 printf("post_full\n");
         printf("Consumer %d dequeue %d from buffer\n", thread_numb, value);
    }
     pthread_exit(0);
 }


 int main(int argc, int **argv) {
     // buffer_index = 0;

	 key_t buff_key, buffind_key, empty_key, full_key, mutex_key;
	 int buff_shmid, buffind_shmid, empty_shmid, full_shmid, mutex_shmid;

	 // int *buff_shm, *buffind_shm;



	 buff_key = 5768;
	 buffind_key = 5678;

	 empty_key = 3412;
	 full_key = 3214;
	 mutex_key = 3124;


	/*******************************************************************************
						 SHARED MEMORY ENSTANTIATION
	*******************************************************************************/
	if( (empty_shmid = shmget(empty_key, sizeof(Csem_t), IPC_CREAT | 0666)) < 0 ) //if creation fails
	{
		perror("buff shmget"); //issue error
		exit(1); //exit
	}

	if( (empty_shm = shmat(empty_shmid, NULL, 0)) == (char *) -1 )
	{
		perror("buff shmat");
		exit(1);
	}

	if( (full_shmid = shmget(full_key, sizeof(Csem_t), IPC_CREAT | 0666)) < 0 ) //if creation fails
	{
		perror("buff shmget"); //issue error
		exit(1); //exit
	}

	if( (full_shm = shmat(full_shmid, NULL, 0)) == (char *) -1 )
	{
		perror("buff shmat");
		exit(1);
	}

	if( (mutex_shmid = shmget(mutex_key, sizeof(Csem_t), IPC_CREAT | 0666)) < 0 ) //if creation fails
	{
		perror("buff shmget"); //issue error
		exit(1); //exit
	}

	if( (mutex_shm = shmat(mutex_shmid, NULL, 0)) == (char *) -1 )
	{
		perror("buff shmat");
		exit(1);
	}






	 if( (buff_shmid = shmget(buff_key, SIZE*4*sizeof(int), IPC_CREAT | 0666)) < 0 ) //if creation fails
	 {
		 perror("buff shmget"); //issue error
		 exit(1); //exit
	 }

	 if( (buffind_shmid = shmget(buffind_key, 4*sizeof(int), IPC_CREAT | 0666)) < 0 ) //if creation fails
	 {
		 perror("buffind shmget"); //issue error
		 exit(1); //exit
	 }

	 if( (buff_shm = shmat(buff_shmid, NULL, 0)) == (char *) -1 )
	 {
		 perror("buff shmat");
		 exit(1);
	 }

	 if( (buffind_shm = shmat(buffind_shmid, NULL, 0)) == (char *) -1 )
	 {
		 perror("buffind shmat");
		 exit(1);
	 }
	 printf("shm init complete\n");

     // pthread_mutex_init(&buffer_mutex, NULL);


	Csem_init(full_shm, // sem_t *sem
	     0, // int pshared. 0 = shared between threads of process,  1 = shared between processes
	     SIZE); // unsigned int value. Initial value
    Csem_init(empty_shm,
             0,
             0);
	Csem_init(mutex_shm,
	      0,
	      1);


		   sleep(5);
     /* full_sem is initialized to buffer size because SIZE number of
        producers can add one element to buffer each. They will wait
        semaphore each time, which will decrement semaphore value.
        empty_sem is initialized to 0, because buffer starts empty and
        consumer cannot take any element from it. They will have to wait
        until producer posts to that semaphore (increments semaphore
        value) */


	 *buffind_shm = 0;
     pthread_t thread[NUMB_THREADS];
     int thread_numb[NUMB_THREADS];
     int i;
     for (i = 0; i < NUMB_THREADS; ) {
         // thread_numb[i] = i;
         // pthread_create(thread + i, // pthread_t *t
         //                NULL, // const pthread_attr_t *attr
         //                producer, // void *(*start_routine) (void *)
         //                thread_numb + i);  // void *arg
         // i++;
         thread_numb[i] = i;
         // playing a bit with thread and thread_numb pointers...
         pthread_create(&thread[i], // pthread_t *t
                        NULL, // const pthread_attr_t *attr
                        consumer, // void *(*start_routine) (void *)
                        &thread_numb[i]);  // void *arg
         i++;
     }

     for (i = 0; i < NUMB_THREADS; i++)
         pthread_join(thread[i], NULL);

	 Csem_destroy(mutex_shm);
     Csem_destroy(full_shm);
     Csem_destroy(empty_shm);

     return 0;
 }
