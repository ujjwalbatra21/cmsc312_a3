//******************************************************************************
//	Engineer:		Christopher Jones
//	Project:		CMSC 312 Assignment 2
//	Date:			03/23/2021
//	Description:	Counting Semaphores using Binary Semaphores
//					CORRECT VERSION
//
//******************************************************************************

#ifndef CSEM_CORRECT_H
#define CSEM_CORRECT_H

#include <semaphore.h>

typedef struct Csem_t Csem_t;
struct Csem_t{
	int val;
	sem_t mutex;
	sem_t gate;
};

void Csem_init( Csem_t *cs, int pshared, int k){
	int value;
	cs->val = k;
	sem_init(&cs->mutex, pshared, 1);
	if(k > 0) value = 1;
	else value = 0;
	sem_init(&cs->gate, pshared, value);

}

void Csem_destroy( Csem_t *cs){
	sem_destroy(&cs->mutex);
	sem_destroy(&cs->gate);
}

void Csem_wait( Csem_t *cs){ //wait
	sem_wait(&cs->gate);
	sem_wait(&cs->mutex);
	cs->val--;
	if(cs->val > 0){
		sem_post(&cs->gate);
	}
	sem_post(&cs->mutex);
}

void Csem_post( Csem_t *cs){ //signal
	sem_wait(&cs->mutex);
	cs->val++;
	if(cs->val == 1){
		sem_post(&cs->gate);
	}
	sem_post(&cs->mutex);
}

#endif
