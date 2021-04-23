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

 #define _SHM_SEM_
//#define _DEBUG1_

#define N 4

#define WRD_NAME "/wrdsem"
#define WRT_NAME "/wrtsem"
#define MUTEX_NAME "/mutexsem"


sem_t *wrt;
sem_t *wrd;
sem_t *mutex;
int *cnt; //starts at 1
int *numserver; //starts at 0
int *numclient;
int numproc = 0;

pthread_t servers[10];


void client(int num){
	printf("created client %d\n", num);
	while(*cnt == 0){}
	printf("starting client %d\n", num);
	int slpdur;

	slpdur = rand() % 10;
	sleep(slpdur);
	printf("client %d slept %d\nending client\n", num, slpdur);
}

void *server(void *num){
	printf("created server %d\n", *((int *)num));
	while(*cnt == 0){}
	printf("starting server %d\n", *((int *)num));

	while(*numclient > 0){
		sleep(1);
		printf("server %d serving\n", *((int *)num));
	}
	printf("exiting server %d\n", *((int *)num));
}


int main()
{



		 key_t cnt_key, numserver_key, wrt_key, wrd_key, mutex_key;

		 int cnt_shmid, numserver_shmid, wrt_shmid, wrd_shmid, mutex_shmid;

		 key_t numclient_key;
		 int numclient_shmid;



		 cnt_key = 5768;
		 numserver_key = 5678;

		 wrt_key = 3412;
		 wrd_key = 3214;
		 mutex_key = 3124;

		 numclient_key = 4132;


		// /*******************************************************************************
		// 					 SHARED MEMORY ENSTANTIATION
		// *******************************************************************************/
		printf("shm_sem\n");
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

		 if( (numserver_shmid = shmget(numserver_key, 4*sizeof(int), IPC_CREAT | 0666)) < 0 ) //if creation fails
		 {
			 perror("numserver shmget"); //issue error
			 exit(1); //exit
		 }

		 if( (cnt = shmat(cnt_shmid, NULL, 0)) == (char *) -1 )
		 {
			 perror("cnt shmat");
			 exit(1);
		 }

		 if( (numserver = shmat(numserver_shmid, NULL, 0)) == (char *) -1 )
		 {
			 perror("numserver shmat");
			 exit(1);
		 }



		 if( (numclient_shmid = shmget(numclient_key, 4*sizeof(int), IPC_CREAT | 0666)) < 0 ) //if creation fails
	 	{
	 		perror("numclient shmget"); //issue error
	 		exit(1); //exit
	 	}

	 	if( (numclient = shmat(numclient_shmid, NULL, 0)) == (char *) -1 )
	 	{
	 		perror("numclient shmat");
	 		exit(1);
	 	}


    sem_init(wrt,0,1);
	sem_init(wrd,0,N);
	sem_init(mutex,0,1);


	*cnt = 0;
	*numserver = 0;
	*numclient = 0;

	int createclient = 5;
	int createserver = 10;

    int a[10] = {1,2,3,4,5,6,7,8,9,10}; //Just used for numbering the producer and consumer


    for(int i = 0; i < createserver; i++) {
		*numserver++;
        pthread_create(&servers[i], NULL, (void *)server, (void *)&a[i]);
    }

	for(int i = 0; i < createclient; i++) {
		if(fork() == 0){
			*numclient = *numclient + 1;
			// printf("before client\n")
			client(i);
			*numclient = *numclient - 1;
			printf("ending client\n");
			// if(*numclient == 1){*numclient = 0;}
			exit(0);
		}
		else{usleep(50000);}
	}
	printf("start\n");
	*cnt = 1;
	// *numclient--;
	sleep(10);

	for(int i = 0; i < createserver; i++) {
	    pthread_join(servers[i], NULL);
		*numserver--;
	}



    sem_destroy(mutex);
    sem_destroy(wrt);
	sem_destroy(wrd);
	shmdt(cnt);
	shmdt(numserver);
	shmdt(numclient);
	shmctl(cnt_shmid, IPC_RMID, NULL);
	shmctl(numserver_shmid, IPC_RMID, NULL);
	shmctl(numclient_shmid, IPC_RMID, NULL);

    return 0;

}
