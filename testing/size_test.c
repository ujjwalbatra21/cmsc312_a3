
#include <stdio.h>
#include <stdlib.h>
// #include <pthread.h>
// #include <fcntl.h>           /* For O_* constants */
// #include <sys/stat.h>        /* For mode constants */
// #include <semaphore.h>
// #include <sys/shm.h>
// #include <signal.h>
// #include "Csem_correct.h"

struct node
{
    char data[32];
    struct node *next;
}*head;
struct node list[25];


int main(void){

    printf("%lu\n", sizeof(struct node));
    printf("%lu\n", sizeof(list));
    printf("%lu\n", sizeof(int));
    // printf("%lu\n", sizeof());
    return 0;
}