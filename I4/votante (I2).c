#include "votante.h"

void close_semaphores(){
    sem_unlink("voting_shared_mutex");
    sem_unlink("voting_candidate");
    sem_unlink("voting_n_votantes");
    sem_unlink("voting_file_mutex");
}

void safe_exit(int EXIT_TYPE) {
    close_semaphores();
    exit(EXIT_TYPE);
}

void sigusr1_handler(int sig){
    usr1_arrived = 1;
}

void sigusr2_handler(int sig){
    usr2_arrived = 1;
}

void sigterm_handler(int sig){ 
    Finalizar_ejecucion = 1;
    if(Allow_term){
        safe_exit(EXIT_SUCCESS);
    }
}

int rand_bool() {
    return rand()%2;
}

char cast_vote() {
    int rand_num = rand_bool();
    return rand_num ? 'Y' : 'N';
}

void votar() {
    FILE *fp = NULL;
    char line[LINE_BUFFER];
    char vote;

    sem_wait(file_mutex);
    if ((fp = fopen(REGISTER, "r+")) == NULL ) {
        printf("CRITICAL ERROR OPENING FILE\n");
        safe_exit(EXIT_FAILURE);
    }
    while (fgets(line, LINE_BUFFER, fp) != NULL) {
        if (atoi(strtok(line, "|")) == getpid()) {
            fseek(fp,-2,SEEK_CUR);
            vote = cast_vote(); 
            fwrite(&vote, sizeof(char), 1, fp);
            fseek(fp, 1, SEEK_CUR);
            break;
        }
    }
    fclose(fp);
    sem_post(file_mutex);

}


void init_semaphores(){
    if((mutex = sem_open("voting_shared_mutex", O_CREAT, S_IRUSR | S_IWUSR ,1))==SEM_FAILED){
        perror("Mutex semaphore error");
        safe_exit(EXIT_FAILURE);
    }
    if((file_mutex = sem_open("voting_file_mutex", O_CREAT, S_IRUSR | S_IWUSR ,1))==SEM_FAILED){
        perror("Mutex semaphore error");
        safe_exit(EXIT_FAILURE);
    }
    if((candidate = sem_open("voting_candidate", O_CREAT, S_IRUSR | S_IWUSR ,1))==SEM_FAILED){
        perror("Candidate semaphore error");
        safe_exit(EXIT_FAILURE);
    }
    if((n_votantes = sem_open("voting_n_votantes", O_CREAT, S_IRUSR | S_IWUSR ,0))==SEM_FAILED){
        perror("N_votantes semaphore error");
        safe_exit(EXIT_FAILURE);
    }
}

int main(int argc, char **argv){
    sigset_t sig1, sig2, oldmask;
    struct sigaction usr1, usr2, term;
    

    /*Set random seed*/
    srand(time(NULL) + getpid());

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
    sigaddset(&term.sa_mask, SIGTERM);

    init_semaphores();
    /*Assign handler to usr1*/
    if(sigaction(SIGUSR1, &usr1, NULL) < 0){
        perror("sigaction error");
        safe_exit(EXIT_FAILURE);
    }
    /*Assign handler to term signal*/
    if(sigaction(SIGTERM, &term, NULL) < 0){
        perror("sigaction error");
        safe_exit(EXIT_FAILURE);
    }
    /*Assign handler to usr2 signal*/
    if(sigaction(SIGUSR2, &usr2, NULL) < 0){
        perror("sigaction error");
        safe_exit(EXIT_FAILURE);
    }
    
    sigfillset(&sig1);
    sigdelset(&sig1, SIGUSR1);
    sigdelset(&sig1, SIGTERM);

    sigfillset(&sig2);
    sigdelset(&sig2, SIGUSR2);
    sigdelset(&sig2, SIGTERM);
    usr1_arrived = 0;
    usr2_arrived = 0;
    /*Avisar de que el votante está listo*/
    sem_post(n_votantes);
    /*Wait for USR1. If USR1 has arrived already, ignore this part*/
    sigprocmask(SIG_BLOCK,&usr1.sa_mask,&oldmask);
    while(usr1_arrived == 0){
        sigsuspend(&oldmask);
    }
    usr1_arrived = 0;
    sigprocmask(SIG_UNBLOCK,&usr1.sa_mask,NULL);
    start_round(sig1, sig2, oldmask);
}

void start_round(sigset_t usr1, sigset_t usr2, sigset_t oldmask){
    int i, voting_finished, n_votes, vote_balance;
    char buffer[LINE_BUFFER];
    struct timespec waiting_time = WAITING_TIME, waiting_round = WAITING_ROUND, waiting_aux = WAITING_AUX;
    char read_vote[2];
    char results[RESULTS_SIZE];
    FILE *f;
    
    while(!Finalizar_ejecucion){
        usr1_arrived = 0;
        usr2_arrived = 0;
        /*Luchar por ser el proceso candidato*/
        ronda++;
        Allow_term = 0;
        if (!sem_trywait(candidate)){
            /*Código del candidato*/
            /*Wait for all voters*/
            for(i = 0; i<processes-1; i++){
                sem_wait(n_votantes);
            }
            /*Send USR2*/
            nanosleep(&waiting_aux,NULL);   /*OMEGASUS*/
            kill(-1*getpgid(getpid()),SIGUSR2);
            voting_finished = 0;
            while(voting_finished == 0){
                /*Comprobar fichero*/
                nanosleep(&waiting_time,NULL);
                sem_wait(file_mutex);
                if(!(f = fopen(REGISTER, "r+"))){
                    printf("Critical error opening file");
                    safe_exit(EXIT_FAILURE);
                }
                strcpy(results, "CANDIDATE ");
                sprintf(buffer, "%d => [ ", getpid());
                strcat(results, buffer);
                n_votes = 0;
                vote_balance=0;
                while (fgets(buffer, LINE_BUFFER, f) != NULL) {
                    strtok(buffer, "|");
                    strcpy(read_vote,strtok(NULL, "") );
                    if (read_vote[0] == 'Y') {
                        sprintf(buffer, "%c ", read_vote[0]);
                        strcat(results, buffer);
                        n_votes+=1;
                        vote_balance+=1;
                    } else if (read_vote[0] == 'N'){
                        sprintf(buffer, "%c ", read_vote[0]);
                        strcat(results, buffer);
                        n_votes+=1;
                        vote_balance-=1;
                    }
                }
                /*Si todos han votado*/
                if(n_votes == (processes-1)){
                    strcat(results, "] => ");
                    if (vote_balance > 0) strcat(results, "Accepted");
                    else strcat(results, "Rejected");
                    voting_finished = 1;
                    printf("%s\n", results);
                    strcpy(read_vote, " ");
                    fseek(f, 0, SEEK_SET);
                    while (fgets(buffer, LINE_BUFFER, f) != NULL) {
                        fseek(f, -2, SEEK_CUR);
                        fwrite(read_vote, sizeof(char), 1, f);
                        fseek(f, 1, SEEK_CUR);
                    }
                    Allow_term = 1;
                    nanosleep(&waiting_round,NULL);
                    /*Correct candidate sempahore*/
                    sem_post(candidate);
                    kill(-1*getpgid(getpid()),SIGUSR1);
                }
                fclose(f);
                sem_post(file_mutex);
                
            }

        }

        else {

            /*Código del votante*/
            
            sem_post(n_votantes);

            /*Señalizar que estoy listo y esperar a usr2*/

            sigprocmask(SIG_BLOCK,&usr2,&oldmask);
            while(usr2_arrived == 0){
                sigsuspend(&oldmask);
            }
            usr2_arrived = 0;
            sigprocmask(SIG_UNBLOCK,&usr2,NULL);

            votar();
            Allow_term = 1;
            /*Wait for USR1. If USR1 has arrived already, ignore this part*/
            sigprocmask(SIG_BLOCK,&usr1,&oldmask);
            while(usr1_arrived == 0){
                sigsuspend(&oldmask);
            }
            usr1_arrived = 0;
            sigprocmask(SIG_UNBLOCK,&usr1,NULL);
        }
    }
    safe_exit(EXIT_SUCCESS);
}
