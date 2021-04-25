#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <semaphore.h>
#include <sys/shm.h>
#include <signal.h>
#include "Csem_correct.h"
#include <time.h>

#define _CMD_ARGS_
#define QFULL_KEY_VAL		3412
#define QEMPTY_KEY_VAL		3214
#define QMUTEX_KEY_VAL		3124
#define QFULL_INIT_VAL		25
#define QEMPTY_INIT_VAL		0
#define QMUTEX_INIT_VAL		1
#define GLOBALQ_LEN			25
#define GLOBALQ_KEY_VAL 	5768
#define NUMSERVER_KEY_VAL	5678
#define NUMCLIENT_KEY_VAL	4132
#define GLOBALQ_INDEX_KEY_VAL	4269
#define AVGWAIT_KEY_VAL  6969

struct node
{
    int data;
	int id;
};
typedef struct node job_t;

pid_t parent_pid;
sem_t *qfull;
sem_t *qempty;
sem_t *qmutex;
job_t *globalq; 
int *numserver; 
int *numclient;
int *globalq_index;
double *avgwait;

key_t globalq_index_key, globalq_key, numserver_key, qfull_key, qempty_key, qmutex_key;

int globalq_index_shmid, globalq_shmid, numserver_shmid, qfull_shmid, qempty_shmid, qmutex_shmid;

key_t numclient_key;
int numclient_shmid;

key_t avgwait_key;
int avgwait_shmid;

pthread_t servers[10];

int createclient = 5;
int createserver = 10;

// int compare(const void * a, const void * b){
// 	job_t *nodeA = (job_t *)a; 
// 	job_t *nodeB = (job_t *)b; 
// 	return (nodeA->data - nodeB->data); 
// }

// void insert_job(job_t *new_job){
// 		//printf("insert: globalq_index = %d\n", *globalq_index);
// 		job_t some_job = *new_job;
// 	if (*globalq_index < GLOBALQ_LEN) 
// 	{
//         globalq[*globalq_index] = some_job;
// 		*globalq_index = *globalq_index + 1;
//     } else {
//         printf("globalq overflow\n");
//     }
// }

void insert_job(job_t *new_job){
	int i, c, d;
	job_t swap; 
	int n = 25; 
	job_t some_job = *new_job;
	*globalq = some_job;
	for (c = 0 ; c < n - 1; c++) {
		for (d = 0 ; d < n - c - 1; d++) {
			if (globalq[d].data > globalq[d+1].data) {
				swap = globalq[d];
				globalq[d] = globalq[d+1];
				globalq[d+1] = swap;
			}
		}
	}
	*globalq_index = *globalq_index + 1;
}


void dequeue_job(job_t *head_job) {
	// printf("dequeing job\n");
	job_t some_job;
	int temp_data, temp_id;
	sem_getvalue(qfull, &temp_data);
	*globalq_index = *globalq_index - 1;
	// some_job = globalq[(*globalq_index)];
	for( int i = 0; i < GLOBALQ_LEN; i++){
		if(globalq[i].data != 0){
			some_job = globalq[i];
			*head_job = some_job;
			some_job.data = 0;
			some_job.id = 0;
			globalq[i] = some_job;
			break;
		}
	}
	


    // } else {
    //     printf("globalq underflow\n");
    // }
}


void client(void){
	int process_num = *numclient;
	*numclient = *numclient + 1;
	time_t t;
	pid_t my_pid = getpid();
	int numjobs, joblen;
	job_t new_job;
	int bruh;
	int client_count = 0;
	double time_taken = 0;
	double total_time = 0;
	
	srand((unsigned)process_num ^ (my_pid<<16));
	numjobs = (rand() % 24) + 1; 	

	// printf("created client %d\n", process_num);
	sleep(1);
	// printf("starting client %d\tnumjobs: %d\n", process_num, numjobs);
	

    int i=0;
    while (i++ < numjobs) {
		clock_t start = clock();
        // sleep(rand() % 10);
		srand((unsigned)process_num ^ (my_pid<<16));
        joblen = (rand() % 900) + 100;	
		new_job.data = joblen; 
		new_job.id = process_num;

        sem_wait(qfull); // sem=0: wait. sem>0: go and decrement it
		
        /* possible race condition here. After this thread wakes up,
           another thread could aqcuire mutex before this one, and add to list.
           Then the list would be full again
           and when this thread tried to insert to buffer there would be
           a buffer overflow error */

        sem_wait(qmutex); /* protecting critical section */
		insert_job(&new_job); 
        sem_post(qmutex);
		// sem_getvalue(qmutex, &bruh);
		// printf("client %d : qmutex = %d\n", process_num, bruh); 
		sem_post(qempty); // post (increment) emptybuffer semaphore
		// sem_getvalue(qempty, &bruh);
		// printf("client %d: qempty = %d\n", process_num, bruh);
        // printf("Client %d added %d:%d to global queue\n", process_num, new_job->id, new_job->data);
		srand(time(NULL) ^ (getpid()<<16));
		t = (rand() % 900000) + 100000 ;
		// printf("client %d sleeping %ld usec after job %d\n", process_num, t, i);

		printf("Producer %d added %d to buffer\n", new_job.id, new_job.data);
		usleep(t);
		clock_t end = clock();
		double time_taken = (double)(end - start)/CLOCKS_PER_SEC; // in seconds 
		total_time = total_time + time_taken;
		client_count++;
    }
	
	// srand(time(NULL) ^ (getpid()<<16));
	// t = rand() % 100000;
	// printf("client %d sleeping %ld sec\n", process_num, t);
	// usleep(t);
	*avgwait = *avgwait + (total_time / (double)client_count);
	// printf("ending client\n");
	*numclient = *numclient - 1;
	// printf("numclients: %d\n", *numclient);
	exit(0);
}


void *server(void *arg){
	int thread_num = *numserver;
	*numserver = *numserver + 1;
	// printf("created server %d\n", thread_num);
	while(*numclient != createclient){}
	//printf("starting server %d\n", thread_num);

    job_t d_job;
    int i=0;
	int server_count = 0;
	double time_taken = 0;
	double total_time = 0;

	while((*globalq_index > 0) || (*numclient > 0)){
		clock_t start = clock();
		sem_wait(qempty);
        sem_wait(qmutex);
        dequeue_job(&d_job);
        sem_post(qmutex);
        sem_post(qfull); // post (increment) fullbuffer semaphore
		printf("Consumer %d dequeue %d, %d from buffer\n", thread_num, d_job.id, d_job.data);
		clock_t end = clock();
		double time_taken = (double)(end - start)/CLOCKS_PER_SEC; // in seconds 
		total_time = total_time + time_taken;
		srand(time(NULL) ^ (rand()<<16));
		usleep((rand() % 900000) + 100000);
		server_count++;
	}
	*avgwait = *avgwait + (total_time / (double)server_count);
	// printf("exiting server %d\n", thread_num);
	*numserver = *numserver - 1;

}


void handle_sigint(int sig){
	if(getpid() == parent_pid){
		printf("\n***Caught signal %d***\n", sig);
		printf("***Killing Child processes***\n");
		printf("***Joining threads***\n");
		for(int i = 0; i < createserver; i++) {
			pthread_cancel(servers[i]);
		}
		printf("***Destorying Sems***\n");
		sem_destroy(qmutex);
		sem_destroy(qfull);
		sem_destroy(qempty);
		printf("***Deallocating Shared Memory***\n");
		shmdt(qfull);
		shmdt(qempty);
		shmdt(qmutex);
		shmdt(globalq);
		shmdt(numserver);
		shmdt(numclient);
		shmctl(qfull_shmid, IPC_RMID, NULL);
		shmctl(qempty_shmid, IPC_RMID, NULL);
		shmctl(qmutex_shmid, IPC_RMID, NULL);
		shmctl(globalq_shmid, IPC_RMID, NULL); 
		shmctl(numserver_shmid, IPC_RMID, NULL);
		shmctl(numclient_shmid, IPC_RMID, NULL);
		printf("***Terminating Program***\n");
		exit(0);
	}
	else{
		//kills any open child processes
		exit(0);
	}
}


void shm_init(void){

	globalq_key = GLOBALQ_KEY_VAL;
	numserver_key = NUMSERVER_KEY_VAL;
	qfull_key = QFULL_KEY_VAL;
	qempty_key = QEMPTY_KEY_VAL;
	qmutex_key = QMUTEX_KEY_VAL;
	numclient_key = NUMCLIENT_KEY_VAL;
	globalq_index_key = GLOBALQ_INDEX_KEY_VAL;
	avgwait_key = AVGWAIT_KEY_VAL;


	// *******************************************************************************
	// 					 SHARED MEMORY ENSTANTIATION
	// *******************************************************************************

	//	shmget - qfull
	if( (qfull_shmid = shmget(qfull_key, sizeof(sem_t), IPC_CREAT | 0666)) < 0 ) //if creation fails
	{
		perror("shmget - qfull"); //issue error
		exit(1); //exit
	}

	//	shmget - qempty
	if( (qempty_shmid = shmget(qempty_key, sizeof(sem_t), IPC_CREAT | 0666)) < 0 ) //if creation fails
	{
		perror("shmget - qempty"); //issue error
		exit(1); //exit
	}

	//	shmget - qmutex
	if( (qmutex_shmid = shmget(qmutex_key, sizeof(sem_t), IPC_CREAT | 0666)) < 0 ) //if creation fails
	{
		perror("shmget - qmutex"); //issue error
		exit(1); //exit
	}

	// //	shmget - globalq
	// if( (globalq_shmid = shmget(globalq_key, GLOBALQ_LEN * sizeof(job_t), IPC_CREAT | 0666)) < 0 ) //if creation fails
	// {
	// 	perror("shmget - globalq"); //issue error
	// 	exit(1); //exit
	// }

	//	shmget - globalq
	if( (globalq_shmid = shmget(globalq_key, (GLOBALQ_LEN * sizeof(job_t)), IPC_CREAT | 0666)) < 0 ) //if creation fails
	{
		perror("shmget - globalq"); //issue error
		exit(1); //exit
	}

	//	shmget - numserver
	if( (numserver_shmid = shmget(numserver_key, 4*sizeof(int), IPC_CREAT | 0666)) < 0 ) //if creation fails
	{
		perror("shmget - numserver"); //issue error
		exit(1); //exit
	}

	//	shmget - numclient
	if( (numclient_shmid = shmget(numclient_key, 4*sizeof(int), IPC_CREAT | 0666)) < 0 ) //if creation fails
	{
		perror("shmget - numclient"); //issue error
		exit(1); //exit
	}

	//	shmget - globalq_index
	if( (globalq_index_shmid = shmget(globalq_index_key, 4*sizeof(int), IPC_CREAT | 0666)) < 0 ) //if creation fails
	{
		perror("shmget - numclient"); //issue error
		exit(1); //exit
	}

	//	shmget - globalq_index
	if( (avgwait_shmid = shmget(avgwait_key, 4*sizeof(float), IPC_CREAT | 0666)) < 0 ) //if creation fails
	{
		perror("shmget - numclient"); //issue error
		exit(1); //exit
	}

	//	shmat - qfull
	if( (qfull = shmat(qfull_shmid, NULL, 0)) == (char *) -1 )
	{
		perror("shmat - qfull");
		exit(1);
	}

	//	shmat - qempty
	if( (qempty = shmat(qempty_shmid, NULL, 0)) == (char *) -1 )
	{
		perror("shmat - qempty");
		exit(1);
	}

	//	shmat - qmutex
	if( (qmutex = shmat(qmutex_shmid, NULL, 0)) == (char *) -1 )
	{
		perror("shmat - qmutex");
		exit(1);
	}

	//	shmat - globalq
	if( (globalq = shmat(globalq_shmid, NULL, 0)) == (char *) -1 )
	{
		perror("shmat - globalq");
		exit(1);
	}

	//	shmat - numserver
	if( (numserver = shmat(numserver_shmid, NULL, 0)) == (char *) -1 )
	{
		perror("shmat - numserver");
		exit(1);
	}

	//	shmat - numclient
	if( (numclient = shmat(numclient_shmid, NULL, 0)) == (char *) -1 )
	{
		perror("shmat - numclient");
		exit(1);
	}
	
	//	shmat - globalq_index
	if( (globalq_index = shmat(globalq_index_shmid, NULL, 0)) == (char *) -1 )
	{
		perror("shmat - globalq_index");
		exit(1);
	}

	//	shmat - globalq_index
	if( (avgwait = shmat(avgwait_shmid, NULL, 0)) == (char *) -1 )
	{
		perror("shmat - globalq_index");
		exit(1);
	}

}

/************************
 * 	SJF IMPLEMENTATION	*
 ************************/

// Compare method to compare 2 job sizes and return shortest job
// Needed for qsort


#ifdef _CMD_ARGS_
int main(int argc, char *argv[]){
	printf("DEBUG w/cmd args\n");
	if(argc < 3){
		printf("Not enough command line arguments\n");
		exit(0);
	}
	else if(argc > 3){
		printf("To many command line arguments\n");
		exit(0);
	}
	clock_t start = clock();
	createclient = atoi(argv[1]);
	createserver = atoi(argv[2]);
#else
int main(void){
	printf("DEBUG no cmd args\n");
#endif
	signal(SIGINT, handle_sigint);
	parent_pid = getpid();

	shm_init();

    sem_init(qfull,1, QFULL_INIT_VAL);
	sem_init(qempty,1, QEMPTY_INIT_VAL);
	sem_init(qmutex,1, QMUTEX_INIT_VAL);

	*numserver = 0;
	*numclient = 0;
	*globalq_index = 0;

    // int a[10] = {1,2,3,4,5,6,7,8,9,10}; //Just used for numbering the producer and consumer

    for(int i = 0; i < createserver; i++) {
		// *numserver = *numserver + 1;
        pthread_create(&servers[i], NULL, (void *)server, (void *)&i);
    }

	//open clients
	for(int i = 0; i < createclient; i++) {
		if(fork() == 0){
			client();

		}
		else{usleep(5000);}
	}
	
	// sleep(10);
	while(*numclient != 0){}
	while(*globalq_index != 0){}
	//printf("im bouta head out\n");
	for(int i = 0; i < createserver; i++) {
			pthread_cancel(servers[i]);
	}

	for(int i = 0; i < createserver; i++) {
	    pthread_join(servers[i], NULL);
		// *numserver = *numserver - 1;
	}
    sem_destroy(qfull);
	sem_destroy(qempty);
	sem_destroy(qmutex);
	shmdt(qfull);
	shmdt(qempty);
	shmdt(qmutex);
	shmdt(globalq);
	shmdt(numserver);
	shmdt(numclient);
	shmctl(qfull_shmid, IPC_RMID, NULL);
	shmctl(qempty_shmid, IPC_RMID, NULL);
	shmctl(qmutex_shmid, IPC_RMID, NULL);
	shmctl(globalq_shmid, IPC_RMID, NULL); 
	shmctl(numserver_shmid, IPC_RMID, NULL);
	shmctl(numclient_shmid, IPC_RMID, NULL);
	clock_t end = clock();
	double time_taken = (double)(end - start)/CLOCKS_PER_SEC; // in seconds 
    printf("time program took %f seconds to execute \n", time_taken); 
	printf("Average wait time is %f\n", *avgwait);

    return 0;

}