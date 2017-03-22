#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition = PTHREAD_COND_INITIALIZER;

volatile int tour = 0;

volatile char* r1 = "R1";
volatile char* r2 = "R2";

volatile bool eR1 = FALSE;
volatile bool eR2 = FALSE;

void* travail (void* _numero) {
    int reste = ((_numero != NULL) ? 0 : 1);
    int i;

    for(i=0; (i < 10); i++) {

        /* attendre mon tour */

        pthread_mutex_lock(&mutex);

        while ((tour % 2) != reste) {
            /* je m'endors car la condition est fausse
               et je libère mutex en une seule opération atomique */
            pthread_cond_wait(&condition, &mutex);
            /* je suis réveillé et j'ai repris mutex */
        }
        pthread_mutex_unlock(&mutex);

        if (reste == 0) printf("Pair (tour=%d)\n", tour);
        else printf("Impair (tour=%d)\n", tour);
        sleep(1);

        /* modifier tour et réveiller les autres */

        pthread_mutex_lock(&mutex);
        tour++;
        pthread_cond_broadcast(&condition);
        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}

void* my_job (void* name_thread) {
    int reste = atoi((char*)name_thread);

    for(int i=0; (i < 10); i++) {

        /* attendre mon tour */
        pthread_mutex_lock(&mutex);
        while ((tour % 4) != reste) {
            /* je m'endors car la condition est fausse
               et je libère mutex en une seule opération atomique */
            pthread_cond_wait(&condition, &mutex);
            /* je suis réveillé et j'ai repris mutex */
        }

        

        pthread_mutex_unlock(&mutex);

        printf("Thread : %d (tour=%d)\n", reste, tour);
        sleep(1);

        /* modifier tour et réveiller les autres */

        pthread_mutex_lock(&mutex);
        tour++;
        pthread_cond_broadcast(&condition);
        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}


// int main (void) {
//     pthread_t filsA, filsB;
//
//     if (pthread_create(&filsA, NULL, travail, NULL )) perror("thread");
//     if (pthread_create(&filsB, NULL, travail, "" )) perror("thread");
//
//     if (pthread_join(filsA, NULL)) perror("pthread_join");
//     if (pthread_join(filsB, NULL)) perror("pthread_join");
//
//     printf("Fin du pere\n") ;
//     return (EXIT_SUCCESS);
// }

int main (void) {
    pthread_t filsA, filsB, filsC, filsD;

    if (pthread_create(&filsA, NULL, my_job, "0" )) perror("thread");
    if (pthread_create(&filsB, NULL, my_job, "1" )) perror("thread");
    if (pthread_create(&filsC, NULL, my_job, "2" )) perror("thread");
    if (pthread_create(&filsD, NULL, my_job, "3" )) perror("thread");

    if (pthread_join(filsA, NULL)) perror("pthread_join");
    if (pthread_join(filsB, NULL)) perror("pthread_join");
    if (pthread_join(filsC, NULL)) perror("pthread_join");
    if (pthread_join(filsD, NULL)) perror("pthread_join");

    printf("Fin du pere\n") ;
    return (EXIT_SUCCESS);
}
