#include "monitor.h"


int main(int argc, char **argv){
    pid_t pid;
    int shm_mem;
    
    if(argc != 2){
        printf("Incorrect arguments. Usage: ./monitor <lag>\n");
        exit(EXIT_FAILURE);
    }

    shm_mem = shm_open("monitor_shmem",O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
    if(shm_mem == -1){
        perror("The memory was already created");
        exit(EXIT_FAILURE);
    }
    
    pid = fork();

    if(pid <0){
        perror("Fork error");
        exit(EXIT_FAILURE);
    } else if(pid > 0){
        execvp("./procesomonitor", &argv[1]);
        perror("execvp");
        exit(EXIT_FAILURE);
    }

}