#include "minero.h"
#include <math.h>

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

    if(argc != 4){
        printf("Not enough arguments passed");
        return -1;
    }
    rounds = atoi(argv[1]);
    threads = atoi(argv[2]);
    target =  atoi(argv[3]);
  
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
        printf("Solution: %ld\tTarget: %ld\n", solution, target);
        target = solution;
    }
  free(thread); 
  free(s);
  exit(EXIT_SUCCESS);
}
