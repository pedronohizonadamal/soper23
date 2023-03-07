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
    int rounds, threads, i, j, res, resMonitor;
    long int target, search_area;
    long int targetMonitor, solutionMonitor;
    Search_space *s;
    pthread_t *thread;
    int monitor_status = 0;

    int pipeToMonitor1[2], pipeToMonitor2[2], pipeToMiner[2];

    if(pipe(pipeToMonitor1) == -1) {
        printf("Pipe error\n");
        exit(EXIT_FAILURE);
    }

    if(pipe(pipeToMonitor2) == -1) {
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
  
    search_area = (long) ceil(((float) POW_LIMIT)/threads);

    /*allocare memory*/
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

        
        if (fork() != 0) {

            // The parent sends an answer to the child
            close(pipeToMonitor1[0]);
            close(pipeToMonitor2[0]);

            write(pipeToMonitor1[1], &solution, sizeof(solution));
            write(pipeToMonitor2[1], &target, sizeof(target));

            close(pipeToMonitor1[1]);
            close(pipeToMonitor2[1]);

            //The parent receives an answer from the child
            close(pipeToMiner[1]);

            read(pipeToMiner[0], &res, sizeof(res));

            close(pipeToMiner[0]);
        }

        else {

            //The child receives an answer and calls check()
            close(pipeToMonitor1[1]);
            close(pipeToMonitor2[1]);

            read(pipeToMonitor1[0], &solutionMonitor, sizeof(solutionMonitor));
            read(pipeToMonitor2[0], &targetMonitor, sizeof(targetMonitor));

            close(pipeToMonitor1[0]);
            close(pipeToMonitor2[0]);

            //The child calls check()
            res=check(solutionMonitor, targetMonitor);

            //The child sends a response back
            close(pipeToMiner[0]);

            write(pipeToMiner[1], &resMonitor, sizeof(resMonitor));

            close(pipeToMiner[1]);

            exit(0);
        }

        if (res == 1) {
            printf("Solution accepted: %08ld --> %08ld\n", target, solution);
        }
        else {
            printf("Solution rejected: %08ld !-> %08ld\n", target, solution);
            printf("The solution has been invalidated\n");
            break;
        }

        target = solution;
        

    }
  free(thread); 
  free(s);
  printf("Monitor exited with status %d\n", monitor_status);
  if (res == 0) exit(EXIT_FAILURE);
  exit(EXIT_SUCCESS);
}
