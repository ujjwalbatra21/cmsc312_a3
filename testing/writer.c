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

 // #define _SHM_SEM_
//#define _DEBUG1_

#define N 4

#define WRD_NAME "/wrdsem"
#define WRT_NAME "/wrtsem"
#define MUTEX_NAME "/mutexsem"

sem_t *wrt;
sem_t *wrd;
sem_t *mutex;
int *cnt; //starts at 1
int *numreader; //starts at 0
int *trigger;

void *writer(void *wno)
{
	//Csem_post(&wrd);
	//Csem_wait(&wrd);
	sem_wait(wrt);
    *cnt = *cnt * 2;
    printf("Writer %d modified cnt to %d\n",(*((int *)wno)),*cnt);
	usleep(50000);
    sem_post(wrt);
	//Csem_post(&wrd);
	//Csem_wait(&wrd);

}



int main()
{
	key_t cnt_key, numreader_key, wrt_key, wrd_key, mutex_key;

	int cnt_shmid, numreader_shmid, wrt_shmid, wrd_shmid, mutex_shmid;

	key_t trigger_key;
	int trigger_shmid;



	cnt_key = 5768;
	numreader_key = 5678;

	wrt_key = 3412;
	wrd_key = 3214;
	mutex_key = 3124;

	trigger_key = 4132;


   /*******************************************************************************
						SHARED MEMORY ENSTANTIATION
   *******************************************************************************/
#ifdef _SHM_SEM_
	printf("shm_sem\n");
   if( (wrt_shmid = shmget(wrt_key, sizeof(sem_t), 0666)) < 0 ) //if creation fails
   {
	   perror("wrt shmget"); //issue error
	   exit(1); //exit
   }

   if( (wrt = shmat(wrt_shmid, NULL, 0)) == (char *) -1 )
   {
	   perror("wrt shmat");
	   exit(1);
   }

   if( (wrd_shmid = shmget(wrd_key, sizeof(sem_t), 0666)) < 0 ) //if creation fails
   {
	   perror("wrd shmget"); //issue error
	   exit(1); //exit
   }

   if( (wrd = shmat(wrd_shmid, NULL, 0)) == (char *) -1 )
   {
	   perror("wrd shmat");
	   exit(1);
   }

   if( (mutex_shmid = shmget(mutex_key, sizeof(sem_t), 0666)) < 0 ) //if creation fails
   {
	   perror("mutex shmget"); //issue error
	   exit(1); //exit
   }

   if( (mutex = shmat(mutex_shmid, NULL, 0)) == (char *) -1 )
   {
	   perror("mutex shmat");
	   exit(1);
   }
#endif





	if( (cnt_shmid = shmget(cnt_key, 4*sizeof(int), 0666)) < 0 ) //if creation fails
	{
		perror("cnt shmget"); //issue error
		exit(1); //exit
	}

	if( (numreader_shmid = shmget(numreader_key, 4*sizeof(int), 0666)) < 0 ) //if creation fails
	{
		perror("numreader shmget"); //issue error
		exit(1); //exit
	}

	if( (cnt = shmat(cnt_shmid, NULL, 0)) == (char *) -1 )
	{
		perror("cnt shmat");
		exit(1);
	}

	if( (numreader = shmat(numreader_shmid, NULL, 0)) == (char *) -1 )
	{
		perror("numreader shmat");
		exit(1);
	}



	if( (trigger_shmid = shmget(trigger_key, 4*sizeof(int), 0666)) < 0 ) //if creation fails
   {
	   perror("trigger shmget"); //issue error
	   exit(1); //exit
   }

   if( (trigger = shmat(trigger_shmid, NULL, 0)) == (char *) -1 )
   {
	   perror("trigger shmat");
	   exit(1);
   }

#ifndef _SHM_SEM_
	printf("sem_open\n");
	wrt = sem_open(WRT_NAME, 0);
   wrd = sem_open(WRD_NAME, 0);
   mutex = sem_open(MUTEX_NAME, 0);
#endif

    pthread_t write[5];


    int a[10] = {1,2,3,4,5,6,7,8,9,10}; //Just used for numbering the producer and consumer
	*trigger = 0;

	while(*trigger == 0){
		// usleep(100);
	}

    for(int i = 0; i < 5; i++) {

        pthread_create(&write[i], NULL, (void *)writer, (void *)&a[i]);
    }

	*trigger = 0;
	while(*trigger == 0){
		// usleep(100);
	}

    for(int i = 0; i < 5; i++) {
        pthread_join(write[i], NULL);
    }


	*trigger = 0;

    return 0;

}
