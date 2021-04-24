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
// #define _LINKED_LIST_
// #define _BASIC_QUEUE_


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

#ifdef _LINKED_LIST_
struct node
{
    int data;
	int id;
    struct node *next;
}*head;
typedef struct node job_t;
#endif

#ifdef _BASIC_QUEUE_

typedef int job_t;

#else
struct node
{
    int data;
	int id;
};
typedef struct node job_t;
#endif

// struct node list[25];

// typedef int buffer_mutex; 
// buffer_t buffer[SIZE]; 
// int buffer_index; 

pid_t parent_pid;

sem_t *qfull;
sem_t *qempty;
sem_t *qmutex;
//struct node globalq_list[GLOBALQ_LEN];
job_t *globalq; 
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

// void insert_job(job_t new_job){
// 	int emptyval;
// 	sem_getvalue(qempty, &emptyval);
// 	if (emptyval < GLOBALQ_LEN) {
//         globalq[(emptyval + 1) * sizeof(job_t)] = new_job;
//     } else {
//         printf("globalq overflow\n");
//     }
// }

// void insert_job(int new_data, int new_id){
// 	printf("inserting job\n");
// 	int emptyval;
// 	job_t *new_job;
// 	sem_getvalue(qempty, &emptyval);
// 	if (emptyval < GLOBALQ_LEN) {
// 		// new_job = (globalq + ((emptyval + 1) * sizeof(job_t)));
// 		new_job = &globalq[emptyval];
// 		new_job->data = new_data;
//         new_job->id = new_id; 
// 	//    (job_t*)(globalq + ((emptyval + 1) * sizeof(job_t)))->data = new_data;
// 	//    (job_t*)(globalq + ((emptyval + 1) * sizeof(job_t)))->id = new_id;
//     } else {
//         printf("globalq overflow\n");
//     }
// }

void insert_job(job_t ins_job){
	printf("inserting job\n");
	int emptyval;
	job_t *new_job;
	sem_getvalue(qempty, &emptyval);
	if (emptyval < GLOBALQ_LEN) {
		// new_job = (globalq + ((emptyval + 1) * sizeof(job_t)));
		new_job = &globalq[emptyval];
		new_job->data = new_data;
        new_job->id = new_id; 
	//    (job_t*)(globalq + ((emptyval + 1) * sizeof(job_t)))->data = new_data;
	//    (job_t*)(globalq + ((emptyval + 1) * sizeof(job_t)))->id = new_id;
    } else {
        printf("globalq overflow\n");
    }
}


void dequeue_job(job_t *head_job) {
	printf("dequeing job\n");
	int emptyval;
	sem_getvalue(qempty, &emptyval);
	job_t *some_job;
	int temp_data, temp_id;
    if (emptyval >= 0) {
		// some_job = (globalq + ((emptyval + 1) * sizeof(job_t)));
		some_job = &globalq[emptyval];
		temp_data = some_job->data;
		temp_id = some_job->id; 
		head_job->data = temp_data;
		head_job->id = temp_id;
		printf("got data\n"); 
    } else {
        printf("globalq underflow\n");
    }
    // return some_job;
}


void client(void){
	int process_num = *numclient;
	*numclient = *numclient + 1;
	time_t t;
	pid_t my_pid = getpid();
	int numjobs, joblen;
	job_t *new_job;
	int bruh;

	srand((unsigned)process_num ^ (my_pid<<16));
	numjobs = (rand() % 24) + 1; 	

	printf("created client %d\n", process_num);
	sleep(1);
	// while(*globalq == 0){}
	printf("starting client %d\tnumjobs: %d\n", process_num, numjobs);
	

    int i=0;
    while (i++ < numjobs) {
        // sleep(rand() % 10);
		srand((unsigned)process_num ^ (my_pid<<16));
        joblen = (rand() % 900) + 100;
		printf("eskideee\n");
		// new_job->data = joblen;
		// new_job->id = my_pid;
		printf("skeet\n");
        sem_wait(qfull); // sem=0: wait. sem>0: go and decrement it
        /* possible race condition here. After this thread wakes up,
           another thread could aqcuire mutex before this one, and add to list.
           Then the list would be full again
           and when this thread tried to insert to buffer there would be
           a buffer overflow error */

		sem_getvalue(qmutex, &bruh);
		printf("client %d : qmutex = %d\n", process_num, bruh);
        sem_wait(qmutex); /* protecting critical section */
		sem_getvalue(qmutex, &bruh);
		printf("client %d : qmutex = %d\n", process_num, bruh);
		
		insert_job(joblen, my_pid);        // insert_job(*new_job);
        sem_post(qmutex);
		sem_getvalue(qmutex, &bruh);
		printf("client %d : qmutex = %d\n", process_num, bruh); 

		sem_post(qempty); // post (increment) emptybuffer semaphore
		sem_getvalue(qempty, &bruh);
		printf("client %d: qempty = %d\n", process_num, bruh);
        printf("Client %d added %d:%d to global queue\n", process_num, new_job->id, new_job->data);
		srand(time(NULL) ^ (getpid()<<16));
		t = (rand() % 900000) + 100000 ;
		printf("client %d sleeping %ld usec after job %d\n", process_num, t, i);
		usleep(t);
    }
	
	// srand(time(NULL) ^ (getpid()<<16));
	// t = rand() % 100000;
	// printf("client %d sleeping %ld sec\n", process_num, t);
	// usleep(t);
	printf("ending client\n");
	*numclient = *numclient - 1;
	printf("numclients: %d", *numclient);
	exit(0);
}


void *server(void *arg){
	int thread_num = *numserver;
	*numserver = *numserver + 1;
	printf("created server %d\n", thread_num);
	while(*numclient == 0){}
	printf("starting server %d\n", thread_num);

    job_t *blow_job;
    int i=0;
	int bruh;

	while(*numclient > 0){
		printf("we in here\n");
		sem_getvalue(qempty, &bruh);
		printf("server: qempty = %d\n", bruh);
		
		sem_wait(qempty);
		// if(sem_trywait(qempty) == 0){
        /* there could be race condition here, that could cause
           buffer underflow error */
		printf("bruh what the fuck\n");
		sem_getvalue(qmutex, &bruh);
		printf("server: qmutex = %d\n", bruh);
        sem_wait(qmutex);
        dequeue_job(blow_job);
        sem_post(qmutex);
        sem_post(qfull); // post (increment) fullbuffer semaphore
        printf("server %d dequeue %d:%d from buffer\n", thread_num, blow_job->id, blow_job->data);
		// usleep(1000);
		// }
		printf("server %d serving\tnumclients: %d\n", thread_num, *numclient);
		srand(time(NULL) ^ (rand()<<16));
		usleep((rand() % 900000) + 100000);
	}
	printf("exiting server %d\n", thread_num);
	*numserver = *numserver - 1;
	// pthread_exit(NULL);
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
	if( (globalq_shmid = shmget(globalq_key, sizeof(job_t), IPC_CREAT | 0666)) < 0 ) //if creation fails
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

//  void noMoreHead(job_t *head_job){
// 	job_t *some_job;
// 	head_job = (job_t *)malloc(sizeof(job_t)); 
 
// 	some_job->data = head_job->data; 
// 	some_job->id = head_job->id; 
// 	free(some_job);
// }

// void insertbuffer(buffer_t value){
// 	int i, n = 0; 
// 	if (buffer_index < GLOBALQ_LEN){
// 		buffer[buffer_index++] = value;  
// 		struct node list[n]->data = value; 
// 		n++;
// 		qsort(list, GLOBALQ_LEN, sizeof(*head), compare); // SJF 
// 	}
// 	else{
// 		printf("Buffer Overflow\n"); 
// 	}

// 	//testing print *NOT NEEDED* 
// 	for (i = 0; i < GLOBALQ_LEN - 1; i++){
// 		printf("%d, %d   ", list[n]-> data, list[n]->id); 
// 	}
// }

// struct node dequeuebuffer(){
// 	struct node *temp; 
// 	int i; 
//  	if (buffer_index > 0){
//  		return(buffer[buffer_index--]);
// 		temp = *head; 
// 		head = head->next; 
// 		free(temp);  
// 		// for (i = 0; i < GLOBALQ_LEN - 1; i++){
// 		// 	list[n]->data = list[n + 1]->data; 
// 		// 	list[n]->id = list[n+1]->id; 
// 		// }
//  	}	
// 	else{
// 		printf("Buffer Underflow"); 
// 	}
// }

/************************
 * 	SJF IMPLEMENTATION	*
 ************************/

// Compare method to compare 2 job sizes and return shortest job
// Needed for qsort
// int compare(const void * a, const void * b){
// 	struct node *nodeA = (struct node *)a; 
// 	struct node *nodeB = (struct node *)b; 
// 	return (nodeA->data - nodeB->data); //returns negative if b > a
// } 

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

    sem_init(qfull,1, QFULL_INIT_VAL);
	sem_init(qempty,1, QEMPTY_INIT_VAL);
	sem_init(qmutex,1, QMUTEX_INIT_VAL);

	*numserver = 0;
	*numclient = 0;

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
	printf("im bouta head out\n");
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

    return 0;

}