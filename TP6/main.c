#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition = PTHREAD_COND_INITIALIZER;
volatile int tour = 0;

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


int main (void) {
    pthread_t filsA, filsB;

    if (pthread_create(&filsA, NULL, travail, NULL )) perror("thread");
    if (pthread_create(&filsB, NULL, travail, "" )) perror("thread");

    if (pthread_join(filsA, NULL)) perror("pthread_join");
    if (pthread_join(filsB, NULL)) perror("pthread_join");

    printf("Fin du pere\n") ;
    return (EXIT_SUCCESS);
}
