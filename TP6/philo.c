#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>

volatile int nb_philo;
volatile bool* fourchettes;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition = PTHREAD_COND_INITIALIZER;

bool isFree(int fourchette){
    return fourchettes[fourchette];
}

void* travail (void* _numero) {
    int reste = atoi((char*) _numero);
    int i;
    int fgauche = (reste - 1 < 0) ? (nb_philo - 1) : (reste - 1);
    int fdroite = (reste + 1 == nb_philo) ? 0 : (reste + 1);
    printf("//--// reste : %d //--// (%d, %d) //--//\n", reste, fgauche, fdroite);
    for(i=0; (i < 10); i++) {


        pthread_mutex_lock(&mutex);
        while (!(isFree(fgauche) && isFree(fdroite))) {
            /* je m'endors car la condition est fausse
               et je libère mutex en une seule opération atomique */
            pthread_cond_wait(&condition, &mutex);
        }
        printf("Le philosophe %d prend les fourchettes\n", reste);

        fourchettes[fgauche] = false;
        fourchettes[fdroite] = false;

        pthread_mutex_unlock(&mutex);

        printf("le philosophe %d commence à manger\n", reste);

        sleep(2);

        printf("Le philosophe %d repose les fourchettes\n", reste);

        pthread_mutex_lock(&mutex);
        fourchettes[fgauche] = true;
        fourchettes[fdroite] = true;
        pthread_cond_broadcast(&condition);
        pthread_mutex_unlock(&mutex);

        printf("Meditation sur la cuisson : %d\n", reste);
        sleep(1);
    }

    return NULL;
}


int main (int argc, char const *argv[]) {
    if(argc < 2){
        printf("USAGE: ./philo {nb philosophes}\n");
        exit(EXIT_FAILURE);
    }
    nb_philo = atoi(argv[1]);
    fourchettes = malloc(sizeof(bool) * nb_philo);
    for(int i = 0 ; i < nb_philo ; i++){
        fourchettes[i] = true;
    }
    pthread_t philosophes[nb_philo];
    char* buffer;
    for(int i = 0 ; i < nb_philo ; i++){
        buffer = malloc(sizeof(char) * 10);
        sprintf(buffer, "%d", i);
        if (pthread_create(&philosophes[i], NULL, travail, buffer)) perror("thread");
    }
    for(int i = 0 ; i < nb_philo ; i++){
        if (pthread_join(philosophes[i], NULL)) perror("pthread_join");
    }
    printf("Fin du pere\n") ;
    return (EXIT_SUCCESS);
}
