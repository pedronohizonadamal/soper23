#include "votante.h"


void sigusr1_handler(int sig){
    usr1_arrived = 1;
}

void sigusr2_handler(int sig){
    usr2_arrived = 1;
}

void sigterm_handler(int sig){ 
    Finalizar_ejecucion = 1;
    printf("Proceso terminado\n");
    exit(EXIT_SUCCESS);
}

void init_semaphores(){
    if((mutex = sem_open("voting_shared_mutex", O_CREAT, S_IRUSR | S_IWUSR ,1))==SEM_FAILED){
        perror("Mutex semaphore error");
        exit(EXIT_FAILURE);
    }
    if((file_mutex = sem_open("voting_file_mutex", O_CREAT, S_IRUSR | S_IWUSR ,1))==SEM_FAILED){
        perror("Mutex semaphore error");
        exit(EXIT_FAILURE);
    }
    if((candidate = sem_open("voting_candidate", O_CREAT, S_IRUSR | S_IWUSR ,1))==SEM_FAILED){
        perror("Candidate semaphore error");
        exit(EXIT_FAILURE);
    }
    if((n_votantes = sem_open("voting_n_votantes", O_CREAT, S_IRUSR | S_IWUSR ,0))==SEM_FAILED){
        perror("N_votantes semaphore error");
        exit(EXIT_FAILURE);
    }
}

void close_semaphores(){
    sem_unlink("voting_shared_mutex");
    sem_unlink("voting_candidate");
    sem_unlink("voting_n_votantes");
    sem_unlink("voting_file_mutex");
}

int main(int argc, char **argv){
    sigset_t sigset;
    struct sigaction usr1, usr2, term;
    int processes;

    if (argc != 1)
    {
        printf("Not enough arguments passed to voter\n");
        exit(EXIT_FAILURE);
    }
    processes = atoi(argv[0]);
    /*Preparar handlers*/
    usr1.sa_handler = sigusr1_handler;
    sigemptyset(&usr1.sa_mask);
    usr1.sa_flags= 0;
    sigaddset(&usr1.sa_mask, SIGUSR1);

    usr2.sa_handler = sigusr2_handler;
    sigemptyset(&usr1.sa_mask);
    usr2.sa_flags= 0;
    sigaddset(&usr2.sa_mask, SIGUSR2);
    
    term.sa_handler = sigterm_handler;
    sigemptyset(&term.sa_mask);
    term.sa_flags= 0;
    sigaddset(&usr2.sa_mask, SIGTERM);

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
    /*Avisar de que el votante está listo*/
    sem_post(n_votantes);
    /*Wait for USR1. If USR1 has arrived already, ignore this part*/
    if(usr1_arrived == 0){
        sigsuspend(&sigset);
    }
    printf("He recibido la señal USR1\n");
    /*Luchar por ser el proceso candidato*/
    if (!sem_trywait(candidate)){
        im_candidate = 1;

    } else {
        im_candidate = 0;

        sigfillset(&sigset);
        sigdelset(&sigset, SIGUSR2);
        sigdelset(&sigset, SIGTERM);

        sem_post(n_votantes);
        /*Señalizar que estoy listo y esperar a usr2*/
        if(usr2_arrived == 0){
            sigsuspend(&sigset);
        }
        printf("USR2 recibida\n");
    }
    /*Esperar a todos los votantes*/

    while(1);
    exit(EXIT_SUCCESS);
    
}