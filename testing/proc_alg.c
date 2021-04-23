#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <semaphore.h>
#include <sys/shm.h>

int main(void){
	int i;
	for(i = 0; i < 5; i++) {
		pid = fork();
		if(pid == 0){
			break;
		}
	}
	printf("exit main")
	return 0;
}
