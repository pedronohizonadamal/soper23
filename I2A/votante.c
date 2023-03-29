#include "votante.h"

int Finalizar_ejecucion = 0;
sem_t *mutex;

void sigusr1_handler(int sig){
    return;
}

void sigterm_handler(int sig){ 
    Finalizar_ejecucion = 1;
    printf("Proceso terminado\n");
    exit(EXIT_SUCCESS);
}

void init_semaphores(){
    if(!(mutex = sem_open("voting_shared_mutex",O_CREAT | O_EXCL , S_IRUSR | S_IWUSR ,1))){
        perror("Semaphore error");
        exit(EXIT_FAILURE);
    }
}

void close_semaphores(){
    sem_unlink("voting_shared_mutex");
}

int main(){
    sigset_t sigset;
    struct sigaction usr1, term;

    usr1.sa_handler = sigusr1_handler;
    sigemptyset(&usr1.sa_mask);
    usr1.sa_flags= 0;
    sigaddset(&usr1.sa_mask, SIGUSR1);
    
    term.sa_handler = sigterm_handler;
    sigemptyset(&term.sa_mask);
    term.sa_flags= 0;

    init_semaphores();
    /*Assign handler to usr1*/
    if(sigaction(SIGUSR1, &usr1, NULL) < 0){
        perror("sigaction error");
        exit(EXIT_FAILURE);
    }
    /*Assign handler to term signal*/
    if(sigaction(SIGTERM, &term, NULL) < 0){
        perror("sigaction error");
        exit(EXIT_FAILURE);
    }

    sigfillset(&sigset);
    sigdelset(&sigset, SIGUSR1);
    sigdelset(&sigset, SIGTERM);

    /*Wait for USR1*/
    sigsuspend(&sigset);
    printf("He recibido la señal USR1\n");
    /*Luchar por ser el proceso candidato. Para ello, usamos un lightswitch*/
    sem_wait(mutex);
    /*Acceder a un contador compartido entre todos los votantes*/
    sem_post(mutex);
    while(1);
    exit(EXIT_SUCCESS);
    
}