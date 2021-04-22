/*
 * server.c
 *
 *  Created on: Apr 20, 2021
 *      Author: chrispy
 */
 #include <signal.h>
 #include <stdio.h>
 #include <stdlib.h>
 #include <pthread.h>
 #include <fcntl.h>           /* For O_* constants */
 #include <sys/stat.h>        /* For mode constants */
 #include <semaphore.h>
 #include <sys/shm.h>

 #define _DEBUG_
//#define _DEBUG1_

#define N 4

sem_t *wrt;
sem_t *wrd;
sem_t *mutex;
int *cnt; //starts at 1
int *numreader; //starts at 0
int *trigger;

pthread_t read[10];

void *reader(void *rno)
{
	sem_wait(wrd);
	//Csem_post(&wrd);
    // Reader acquire the lock before modifying numreader
    sem_wait(mutex);
    *numreader++;
    if(*numreader == 1) {
        sem_wait(wrt); // If this id the first reader, then it will block the writer
    }
    sem_post(mutex);
	for(int i = 0; i < 10; i++){
    // Reading Section
    printf("Reader %d: read cnt as %d\n",*((int *)rno),*cnt);
	usleep(10000);
	}
    // Reader acquire the lock before modifying numreader
    sem_wait(mutex);
    *numreader--;
    if(*numreader == 0) {
        sem_post(wrt); // If this is the last reader, it will wake up the writer.
    }
    sem_post(mutex);

 	sem_post(wrd);
	//Csem_wait(&wrd);
}
void handle_sigint(int sig){
	printf("\nCaught signal %d\n", sig);

	for(int i = 0; i < 10; i++) {
		printf("Joining Thread %d\n", i);
        pthread_join(read[i], NULL);
    }
	printf("Destorying Sems\n");
    sem_destroy(mutex);
    sem_destroy(wrt);
	sem_destroy(wrd);
	exit(1);
}

int main()
{

	signal(SIGINT, handle_sigint);

    pthread_t read[10];

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
		if( (wrt_shmid = shmget(wrt_key, sizeof(sem_t), IPC_CREAT | 0666)) < 0 ) //if creation fails
		{
			perror("wrt shmget"); //issue error
			exit(1); //exit
		}

		if( (wrt = shmat(wrt_shmid, NULL, 0)) == (char *) -1 )
		{
			perror("wrt shmat");
			exit(1);
		}

		if( (wrd_shmid = shmget(wrd_key, sizeof(sem_t), IPC_CREAT | 0666)) < 0 ) //if creation fails
		{
			perror("wrd shmget"); //issue error
			exit(1); //exit
		}

		if( (wrd = shmat(wrd_shmid, NULL, 0)) == (char *) -1 )
		{
			perror("wrd shmat");
			exit(1);
		}

		if( (mutex_shmid = shmget(mutex_key, sizeof(sem_t), IPC_CREAT | 0666)) < 0 ) //if creation fails
		{
			perror("mutex shmget"); //issue error
			exit(1); //exit
		}

		if( (mutex = shmat(mutex_shmid, NULL, 0)) == (char *) -1 )
		{
			perror("mutex shmat");
			exit(1);
		}






		 if( (cnt_shmid = shmget(cnt_key, 4*sizeof(int), IPC_CREAT | 0666)) < 0 ) //if creation fails
		 {
			 perror("cnt shmget"); //issue error
			 exit(1); //exit
		 }

		 if( (numreader_shmid = shmget(numreader_key, 4*sizeof(int), IPC_CREAT | 0666)) < 0 ) //if creation fails
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



		 if( (trigger_shmid = shmget(trigger_key, 4*sizeof(int), IPC_CREAT | 0666)) < 0 ) //if creation fails
	 	{
	 		perror("trigger shmget"); //issue error
	 		exit(1); //exit
	 	}

	 	if( (trigger = shmat(trigger_shmid, NULL, 0)) == (char *) -1 )
	 	{
	 		perror("trigger shmat");
	 		exit(1);
	 	}

    // pthread_mutex_init(&mutex, NULL);
    sem_init(wrt,0,1);
	sem_init(wrd,0,N);
	sem_init(mutex,0,1);

	*cnt = 1;
	*numreader = 0;
	*trigger = 1;

    int a[10] = {1,2,3,4,5,6,7,8,9,10}; //Just used for numbering the producer and consumer

	while(*trigger == 1){
		// usleep(100);
	}

    for(int i = 0; i < 10; i++) {

        pthread_create(&read[i], NULL, (void *)reader, (void *)&a[i]);
    }
	*trigger = 1;
	while(*trigger == 1){
		// usleep(100);
	}

    for(int i = 0; i < 10; i++) {
        pthread_join(read[i], NULL);
    }

	*trigger = 1;
	while(*trigger == 1){
		// usleep(100);
	}

    sem_destroy(mutex);
    sem_destroy(wrt);
	sem_destroy(wrd);

    return 0;

}
