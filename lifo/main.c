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

#define _CMD_ARGS_

#define GLOBALQ_LEN			25

#define QFULL_KEY_VAL		3412
#define QEMPTY_KEY_VAL		3214
#define QMUTEX_KEY_VAL		3124
#define QFULL_INIT_VAL		5
#define QEMPTY_INIT_VAL		0
#define QMUTEX_INIT_VAL		1

#define GLOBALQ_KEY_VAL 	5768
#define NUMSERVER_KEY_VAL	5678
#define NUMCLIENT_KEY_VAL	4132

struct node
{
    int data;
    struct node *next;
}*head;
// struct node list[25];


pid_t parent_pid;

Csem_t *qfull;
Csem_t *qempty;
Csem_t *qmutex;
//struct node globalq_list[GLOBALQ_LEN];
struct node *globalq; 
int *numserver; 
int *numclient;
// int numproc = 0;

key_t globalq_key, numserver_key, qfull_key, qempty_key, qmutex_key;

int globalq_shmid, numserver_shmid, qfull_shmid, qempty_shmid, qmutex_shmid;

key_t numclient_key;
int numclient_shmid;

pthread_t servers[10];

int createclient = 5;
int createserver = 10;
// int createclient;
// int createserver;


void client(int num){
	*numclient = *numclient + 1;
	printf("created client %d\n", num);
	sleep(1);
	// while(*globalq == 0){}
	printf("starting client %d\n", num);
	int slpdur;

	slpdur = rand() % 10;
	printf("client %d sleeping %d sec\n", num, slpdur);
	sleep(slpdur);
	printf("ending client\n");
	sleep(10);
	*numclient = *numclient - 1;
	exit(0);
}

// void *server(void *num){
// 	printf("created server %d\n", *((int *)num));
// 	while(*globalq == 0){}
// 	printf("starting server %d\n", *((int *)num));

// 	while(*numclient > 0){
// 		sleep(1);
// 		printf("server %d serving\n", *((int *)num));
// 	}
// 	printf("exiting server %d\n", *((int *)num));
// }

void *server(void *num){
	int thread_num = *((int *)num);
	printf("created server %d\n", thread_num);
	while(*numclient == 0){}
	printf("starting server %d\n", thread_num);

	while(*numclient > 0){
		sleep(1);
		printf("server %d serving\tnumclients: %d\n", thread_num, *numclient);
	}
	printf("exiting server %d\n", thread_num);
}

void handle_sigint(int sig){
	if(getpid() == parent_pid){
		printf("\n***Caught signal %d***\n", sig);
		printf("***Killing Child processes***\n");
		printf("***Joining threads***\n");
		// *numclients = 0;
		for(int i = 0; i < createserver; i++) {
			pthread_join(servers[i], NULL);
			*numserver--;
		}
		printf("***Destorying Sems***\n");
		Csem_destroy(qmutex);
		Csem_destroy(qfull);
		Csem_destroy(qempty);
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
		*numclient--;
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


	// *******************************************************************************
	// 					 SHARED MEMORY ENSTANTIATION
	// *******************************************************************************

	//	shmget - qfull
	if( (qfull_shmid = shmget(qfull_key, sizeof(Csem_t), IPC_CREAT | 0666)) < 0 ) //if creation fails
	{
		perror("shmget - qfull"); //issue error
		exit(1); //exit
	}

	//	shmget - qempty
	if( (qempty_shmid = shmget(qempty_key, sizeof(Csem_t), IPC_CREAT | 0666)) < 0 ) //if creation fails
	{
		perror("shmget - qempty"); //issue error
		exit(1); //exit
	}

	//	shmget - qmutex
	if( (qmutex_shmid = shmget(qmutex_key, sizeof(Csem_t), IPC_CREAT | 0666)) < 0 ) //if creation fails
	{
		perror("shmget - qmutex"); //issue error
		exit(1); //exit
	}

	//	shmget - globalq
	if( (globalq_shmid = shmget(globalq_key, GLOBALQ_LEN * sizeof(struct node), IPC_CREAT | 0666)) < 0 ) //if creation fails
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
}

//typedef int buffer_mutex; 
//buffer_t buffer[SIZE]; 
//int buffer_index; 
/*
 * THESE 3 VARIABLES NEED TO BE IN SHARED MEMORY
 *	  pthread_mutex_t buffer_mutex; 
 *	  Csem_t full_sem
 *    semt_t empty_sem
 */

/*
    void insertbuffer (buffer_t val)
		if buffer_index < size
			buffer[buffer_index++] = value
			int n = 0; 
			struct node list[n] = value
			n++
			qsort
		else
			"Buffer Overflow"
 */

/*
	void dequeuebuffer()
		if buffer_index > 0
			return buffer[--buffer_index];
			
		else
			"Buffer underflow"	
 */

/************************
 * 	SJF IMPLEMENTATION	*
 ************************/

// Compare method to compare 2 job sizes and return shortest job
// Needed for qsort
int compare(const void * a, const void * b){
	struct node *nodeA = (struct node *)a; 
	struct node *nodeB = (struct node *)b; 
	return (nodeA->data - nodeB->data); //returns negative if b > a
} 
//qsort(list, 25, sizeof(*head), compare); 

#ifdef _CMD_ARGS_
int main(int argc, char *argv[]){
	printf("DEBUG w/cmd args\n");
	if(argc < 3){
		printf("Not enough command line arguments\n");
		exit(0);
	}
	else if(argc > 3){
		printf("too command line arguments\n");
		exit(0);
	}
	createclient = atoi(argv[1]);
	createserver = atoi(argv[2]);
#else
int main(void){
	printf("DEBUG no cmd args\n");
#endif
	signal(SIGINT, handle_sigint);
	parent_pid = getpid();

	shm_init();

    Csem_init(qfull,0, QFULL_INIT_VAL);
	Csem_init(qempty,0, QEMPTY_INIT_VAL);
	Csem_init(qmutex,0, QMUTEX_INIT_VAL);

	*numserver = 0;
	*numclient = 0;

	

    // int a[10] = {1,2,3,4,5,6,7,8,9,10}; //Just used for numbering the producer and consumer


    for(int i = 0; i < createserver; i++) {
		*numserver = *numserver + 1;
        pthread_create(&servers[i], NULL, (void *)server, (void *)&i);
    }

	//open clients
	for(int i = 0; i < createclient; i++) {
		if(fork() == 0){
			// *numclient = *numclient + 1;
			// printf("before client\n")
			client(i);
			// *numclient = *numclient - 1;
			// printf("ending client\n");
			// exit(0);
		}
		else{usleep(5000);}
	}
	
	sleep(10);

	for(int i = 0; i < createserver; i++) {
	    pthread_join(servers[i], NULL);
		*numserver = *numserver - 1;
	}
    printf("we goin dis way\n");
    Csem_destroy(qfull);
	Csem_destroy(qempty);
	Csem_destroy(qmutex);
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

    return 0;

}
