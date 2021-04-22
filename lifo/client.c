/*
 * client.c
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


 void *producer(void *thread_n) {
     int thread_numb = *(int *)thread_n;
     int value;
     int i=0;
	 printf("Producer thread #%d created\n", thread_numb);
     while (i++ < PRODUCER_LOOPS) {
		 printf("#%d entered loop\n", thread_numb);
         sleep(rand() % 10);
         value = rand() % 100;
         sem_wait(full_shm); // sem=0: wait. sem>0: go and decrement it
		 printf("#%d full_sem waited\n", thread_numb);
         /* possible race condition here. After this thread wakes up,
            another thread could aqcuire mutex before this one, and add to list.
            Then the list would be full again
            and when this thread tried to insert to buffer there would be
            a buffer overflow error */
         sem_wait(mutex_shm); /* protecting critical section */
		 printf("#%d mutex_sem waited\n", thread_numb);
         insertbuffer(value);
         sem_post(mutex_shm);
		 printf("#%d mutex_sem posted\n", thread_numb);
         sem_post(empty_shm); // post (increment) emptybuffer semaphore
		 printf("#%d empty_sem posted\n", thread_numb);
         printf("Producer %d added %d to buffer\n", thread_numb, value);
     }
     pthread_exit(0);
 }

//  void *writer(void *wno)
// {
// 	//Csem_post(&wrd);
// 	//Csem_wait(&wrd);
// 	sem_wait(&wrt);
//     cnt = cnt*2;
//     printf("Writer %d modified cnt to %d\n",(*((int *)wno)),cnt);
// 	//usleep(500);
//     sem_post(&wrt);
// 	//Csem_post(&wrd);
// 	//Csem_wait(&wrd);
//
// }


 int main(int argc, int **argv) {
	 //buffer_index = 0;

	 key_t buff_key, buffind_key, empty_key, full_key, mutex_key;
	int buff_shmid, buffind_shmid, empty_shmid, full_shmid, mutex_shmid;

	 // int *buff_shm, *buffind_shm;

	 key_t trigger_key;
	 int trigger_shmid;



	 buff_key = 5768;
	 buffind_key = 5678;

	 /*******************************************************************************
	 					SHARED MEMORY ENSTANTIATION
	 *******************************************************************************/

	 empty_key = 3412;
	 full_key = 3214;
	 mutex_key = 3124;

	 trigger_key = 4132;


	/*******************************************************************************
						 SHARED MEMORY ENSTANTIATION
	*******************************************************************************/
	if( (empty_shmid = shmget(empty_key, sizeof(sem_t), 0666)) < 0 ) //if creation fails
	{
		perror("buff shmget"); //issue error
		exit(1); //exit
	}

	if( (empty_shm = shmat(empty_shmid, NULL, 0)) == (char *) -1 )
	{
		perror("buff shmat");
		exit(1);
	}

	if( (full_shmid = shmget(full_key, sizeof(sem_t), 0666)) < 0 ) //if creation fails
	{
		perror("buff shmget"); //issue error
		exit(1); //exit
	}

	if( (full_shm = shmat(full_shmid, NULL, 0)) == (char *) -1 )
	{
		perror("buff shmat");
		exit(1);
	}

	if( (mutex_shmid = shmget(mutex_key, sizeof(sem_t), 0666)) < 0 ) //if creation fails
	{
		perror("buff shmget"); //issue error
		exit(1); //exit
	}

	if( (mutex_shm = shmat(mutex_shmid, NULL, 0)) == (char *) -1 )
	{
		perror("buff shmat");
		exit(1);
	}




	 if( (buff_shmid = shmget(buff_key, SIZE*4*sizeof(int), 0666)) < 0 ) //if creation fails
	 {
	 	perror("buff shmget"); //issue error
	 	exit(1); //exit
	 }

	 if( (buffind_shmid = shmget(buffind_key, 4*sizeof(int), 0666)) < 0 ) //if creation fails
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

	 if( (trigger_shmid = shmget(trigger_key, 4*sizeof(int), 0666)) < 0 ) //if creation fails
	{
		perror("trigger shmget"); //issue error
		exit(1); //exit
	}

	if( (trigger_shm = shmat(trigger_shmid, NULL, 0)) == (char *) -1 )
	{
		perror("trigger shmat");
		exit(1);
	}

	 // pthread_mutex_init(&buffer_mutex, NULL);
	 printf("shm init complete\n");


	//  full_sem = sem_open(name_full, // sem_t *sem
	//  		O_RDWR);
	// if()
	//  empty_sem =sem_open(name_empty,
	// 	 	O_RDWR);
	//  mutex_sem =sem_open(name_mutex,
	//  	  	O_RDWR);

	sleep(2);

     /* full_sem is initialized to buffer size because SIZE number of
        producers can add one element to buffer each. They will wait
        semaphore each time, which will decrement semaphore value.
        empty_sem is initialized to 0, because buffer starts empty and
        consumer cannot take any element from it. They will have to wait
        until producer posts to that semaphore (increments semaphore
        value) */
     pthread_t thread[NUMB_THREADS];
     int thread_numb[NUMB_THREADS];
     int i;
     for (i = 0; i < NUMB_THREADS; ) {
		 while(*trigger_shm == 1){
			usleep(100);
		}
         thread_numb[i] = i;
         pthread_create(thread + i, // pthread_t *t
                        NULL, // const pthread_attr_t *attr
                        producer, // void *(*start_routine) (void *)
                        thread_numb + i);  // void *arg
         i++;
		 *trigger_shm = 1;
     }

     for (i = 0; i < NUMB_THREADS; i++)
         pthread_join(thread[i], NULL);

	 sem_destroy(mutex_shm);
     sem_destroy(full_shm);
     sem_destroy(empty_shm);

     return 0;
 }
