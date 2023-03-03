#include "minero.h"

struct _Search_space
{
  long int lower;
  long int upper;
  long int target;

};

void search(Search_space *s){
    long int i;
    for(i = s->lower; i<s->upper && solution==-1; i++){
        if (pow_hash(i) == s->target){
            solution = i;
            pthread_exit(i);
        }
    }
    pthread_exit(-1);
}

int main(int argc, char **argv){
    int rounds, threads, i, j;
    long int target, search_area;
    Search_space *s;
    pthread_t *thread;

    if(argc != 4){
        printf("Not enough arguments passed");
        return -1;
    }
    rounds = (int) argv[1];
    threads = (int) argv[2];
    target = (int) argv[3];
    
    for(i = 0; i<rounds; i++){
        solution = -1;
        search_area = (long) ceil(((float) POW_LIMIT)/threads);
        if (!(thread = (pthread_t *) calloc(sizeof(pthread_t),threads))){
            printf("Error allocating memory");
            return -1;
        }
        /*Crear hilos*/
        for(j = 0; j<threads; j++){
            s->lower = search_area*j;
            s->upper = search_area*(j+1);
            s->target = target;
            if (pthread_create(&thread[j], NULL, search, s) != 0){
                exit(EXIT_FAILURE);
            }
        }
        /*Esperar hilos*/
        for(j = 0; j<threads; j++){
            if (pthread_join(thread[j], NULL)!=0){
                exit(EXIT_FAILURE);
            }
        }
    }
}