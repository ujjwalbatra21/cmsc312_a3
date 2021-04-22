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
int *trigger_shm;


// Csem_t *empty_shm, *full_shm, *mutex_shm;
sem_t *empty_shm, *full_shm, *mutex_shm;
// sem_t *empty_sem, *full_sem, *mutex_sem;


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
	 printf("Consumer thread #%d created\n", thread_numb);
     while (i++ < PRODUCER_LOOPS) {
		 printf("#%d entered loop\n", thread_numb);
         sem_wait(empty_shm);
		 printf("#%d empty_sem waited\n", thread_numb);
         /* there could be race condition here, that could cause
            buffer underflow error */
         sem_wait(mutex_shm);
		 printf("#%d mutex_sem waited\n", thread_numb);
         value = dequeuebuffer(value);
         sem_post(mutex_shm);
		 printf("#%d mutex_sem posted\n", thread_numb);
         sem_post(full_shm); // post (increment) fullbuffer semaphore
		 printf("#%d full_sem posted\n", thread_numb);
         printf("Consumer %d dequeue %d from buffer\n", thread_numb, value);
    }
     pthread_exit(0);
 }

//  void *reader(void *rno)
// {
// 	Csem_wait(&wrd);
// 	//Csem_post(&wrd);
//     // Reader acquire the lock before modifying numreader
//     pthread_mutex_lock(&mutex);
//     numreader++;
//     if(numreader == 1) {
//         Csem_wait(&wrt); // If this id the first reader, then it will block the writer
//     }
//     pthread_mutex_unlock(&mutex);
// 	for(int i = 0; i < 10; i++){
//     // Reading Section
//     printf("Reader %d: read cnt as %d\n",*((int *)rno),cnt);
// 	//usleep(10000);
// 	}
//     // Reader acquire the lock before modifying numreader
//     pthread_mutex_lock(&mutex);
//     numreader--;
//     if(numreader == 0) {
//         Csem_post(&wrt); // If this is the last reader, it will wake up the writer.
//     }
//     pthread_mutex_unlock(&mutex);
//
//  	Csem_post(&wrd);
// 	//Csem_wait(&wrd);
// }


 int main(int argc, int **argv) {
     // buffer_index = 0;

	 key_t buff_key, buffind_key, empty_key, full_key, mutex_key;

	 int buff_shmid, buffind_shmid, empty_shmid, full_shmid, mutex_shmid;

	 key_t trigger_key;
	 int trigger_shmid;

	 // int *buff_shm, *buffind_shm;



	 buff_key = 5768;
	 buffind_key = 5678;

	 empty_key = 3412;
	 full_key = 3214;
	 mutex_key = 3124;

	 trigger_key = 4132;

	//  Csem_init(full_sem, // sem_t *sem
 	//      0, // int pshared. 0 = shared between threads of process,  1 = shared between processes
 	//      SIZE); // unsigned int value. Initial value
    //  Csem_init(empty_sem,
    //           0,
    //           0);
 	// Csem_init(mutex_sem,
 	//       0,
 	//       1);


	/*******************************************************************************
						 SHARED MEMORY ENSTANTIATION
	*******************************************************************************/
	if( (empty_shmid = shmget(empty_key, sizeof(sem_t), IPC_CREAT | 0666)) < 0 ) //if creation fails
	{
		perror("buff shmget"); //issue error
		exit(1); //exit
	}

	if( (empty_shm = shmat(empty_shmid, NULL, 0)) == (char *) -1 )
	{
		perror("buff shmat");
		exit(1);
	}

	if( (full_shmid = shmget(full_key, sizeof(sem_t), IPC_CREAT | 0666)) < 0 ) //if creation fails
	{
		perror("buff shmget"); //issue error
		exit(1); //exit
	}

	if( (full_shm = shmat(full_shmid, NULL, 0)) == (char *) -1 )
	{
		perror("buff shmat");
		exit(1);
	}

	if( (mutex_shmid = shmget(mutex_key, sizeof(sem_t), IPC_CREAT | 0666)) < 0 ) //if creation fails
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



	 if( (trigger_shmid = shmget(trigger_key, 4*sizeof(int), IPC_CREAT | 0666)) < 0 ) //if creation fails
 	{
 		perror("trigger shmget"); //issue error
 		exit(1); //exit
 	}

 	if( (trigger_shm = shmat(trigger_shmid, NULL, 0)) == (char *) -1 )
 	{
 		perror("trigger shmat");
 		exit(1);
 	}


	 printf("shm init complete\n");

     // pthread_mutex_init(&buffer_mutex, NULL);
	 // *empty_shm = empty_sem;
	 // *full_shm = full_sem;
	 // *mutex_shm = mutex_sem;


	sem_init(full_shm, // sem_t *sem
	     0, // int pshared. 0 = shared between threads of process,  1 = shared between processes
	     SIZE); // unsigned int value. Initial value
    sem_init(empty_shm,
             0,
             0);
	sem_init(mutex_shm,
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

	*trigger_shm = 0;
	 *buffind_shm = 0;
     pthread_t thread[NUMB_THREADS];
     int thread_numb[NUMB_THREADS];
     int i;
     for (i = 0; i < NUMB_THREADS; ) {
		 while(*trigger_shm == 0){
			 usleep(100);
		 }

         thread_numb[i] = i;
         // playing a bit with thread and thread_numb pointers...
         pthread_create(&thread[i], // pthread_t *t
                        NULL, // const pthread_attr_t *attr
                        consumer, // void *(*start_routine) (void *)
                        &thread_numb[i]);  // void *arg
         i++;
		 *trigger_shm = 0;
     }

     for (i = 0; i < NUMB_THREADS; i++)
         pthread_join(thread[i], NULL);

	 sem_destroy(mutex_shm);
     sem_destroy(full_shm);
     sem_destroy(empty_shm);

     return 0;
 }
