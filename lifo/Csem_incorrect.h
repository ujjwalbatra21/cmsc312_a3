#ifndef CSEM_INCORRECT_H
#define CSEM_INCORRECT_H

#include <semaphore.h>

typedef struct Csem_t Csem_t;
struct Csem_t{
	int val;
	sem_t mutex;
	sem_t gate;
};


void Csem_init( Csem_t *cs, int pshared, int k){
	cs->val = k;
	sem_init(&cs->mutex, pshared, 1);
	sem_init(&cs->gate, pshared, 0);
}

void Csem_destroy( Csem_t *cs){
	sem_destroy(&cs->mutex);
	sem_destroy(&cs->gate);
}

void Csem_wait( Csem_t *cs){ //wait
//	sem_wait(&cs->gate);
	sem_wait(&cs->mutex);
	cs->val--;
	if(cs->val < 0){
		sem_post(&cs->mutex);
		sem_wait(&cs->gate);
	}
	else{
		sem_post(&cs->mutex);
	}

}

void Csem_post( Csem_t *cs){ //signal
	sem_wait(&cs->mutex);
	cs->val++;
	if(cs->val <= 0){
		sem_post(&cs->gate);
	}
	sem_post(&cs->mutex);
}
