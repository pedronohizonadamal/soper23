#include "minero.h"

struct _Search_space
{
  long int lower;
  long int upper;
  long int target;

};

void *search(void *s){
    long int i;
    int not_found = -1;
    Search_space search = *(Search_space*)s;
    
    for(i = search.lower; i<search.upper && solution==-1; i++){
        if (pow_hash(i) == search.target){
            solution = i;
            pthread_exit((void *) &i);
        }
    }
    pthread_exit((void *) &not_found);
} 

int main(int argc, char **argv){
    int rounds, threads, i, j;
    long int target, search_area;
    Search_space *s;
    pthread_t *thread;
    int monitor_status = 0;
    char confirmation_code;
    int exit_code = EXIT_SUCCESS;

    int pipeToMonitor[2], pipeToMiner[2];

    if(pipe(pipeToMonitor) == -1) {
        printf("Pipe error\n");
        exit(EXIT_FAILURE);
    }
    if(pipe(pipeToMiner) == -1) {
        printf("Pipe error\n");
        exit(EXIT_FAILURE);
    }

    /*nota: esto solo funciona si llamamos a minero desde un execv*/
    if(argc != 3){
        printf("Not enough arguments passed to miner; %d arguments passed\nUsage: <<rounds>> <<threads>> <<target>>\n", argc);
        return -1;
    }

    target = atoi(argv[0]);
    rounds = atoi(argv[1]);
    threads = atoi(argv[2]);

    if (fork() == 0) {
        /*Monitor process*/
        close(pipeToMonitor[1]);
        close(pipeToMiner[0]);
        do{
            if (read(pipeToMonitor[0],&solution,sizeof(solution)) == 0){
                break;
            }
            if (read(pipeToMonitor[0],&target,sizeof(target)) == 0){
                break;
            }

            if(pow_hash(solution) == target){
                printf("Solution accepted: %08ld --> %08ld\n", target, solution);
                confirmation_code = 'O';
                
            } else {
                confirmation_code = 'E';
                printf("Solution rejected: %08ld !-> %08ld\n", target, solution);
            }
            write(pipeToMiner[1],&confirmation_code,sizeof(char));
        } while (1);

        close(pipeToMonitor[0]);
        close(pipeToMiner[1]);

        exit(EXIT_SUCCESS);
    } else {
        /*Miner process*/
        // Close unused access
        close(pipeToMonitor[0]);
        close(pipeToMiner[1]);
    }

    search_area = (long) ceil(((float) POW_LIMIT)/threads);

    /*allocate memory*/
    if (!(thread = (pthread_t *) malloc(sizeof(pthread_t)*threads))){
        printf("Error allocating memory");
        return -1;
    }
    if (!(s = (Search_space *) malloc(sizeof(Search_space)*threads))){
        free(thread);
        printf("Error allocating memory");
        return -1;
    }

    for(i = 0; i<rounds; i++){
        solution = -1;
        
        /*Crear hilos*/
        for(j = 0; j<threads; j++){
            s[j].lower = search_area*j;
            s[j].upper = search_area*(j+1);
            s[j].target = target;
            if (pthread_create(&thread[j], NULL, search, &s[j]) != 0){
                free(s);
                free(thread);
                exit(EXIT_FAILURE);
            }
        }
        /*Esperar hilos*/
        for(j = 0; j<threads; j++){
            if (pthread_join(thread[j], NULL)!=0){
                free(s);
                free(thread);
                exit(EXIT_FAILURE);
            }
        }
        
        /*Escribir a monitor con la solución*/
        write(pipeToMonitor[1], &solution, sizeof(solution));
        write(pipeToMonitor[1], &target, sizeof(target));

        /*Esperar repuesta*/
        if (read(pipeToMiner[0], &confirmation_code, sizeof(confirmation_code)) == 0){
            printf("Error reading from pipe\n");
            exit(EXIT_FAILURE);
        }
        if(confirmation_code == 'O'){
            target = solution;
        } else {
            printf("The solution has been invalidated\n");
            exit_code = EXIT_FAILURE;
            break;
        }

    }
    /*free memory, close pipes and wait*/
  close(pipeToMiner[0]);
  close(pipeToMonitor[1]);
  free(thread); 
  free(s);
  wait(&monitor_status);
  printf("Monitor exited with status %d\n", WEXITSTATUS(monitor_status));
  exit(exit_code);
}
