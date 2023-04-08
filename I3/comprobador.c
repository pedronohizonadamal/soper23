#include "monitor.h"


int main(int argc, char **argv){
    int lag;
    int shmid;
    char *shm, *s, c;

    if (argc != 1) {
        printf("Not enough arguments passed to comprobador\n");
        exit(EXIT_FAILURE);
    }
    lag = atoi(argv[0]);

    /*
    if ((shmid = shmget(KEY,SHMSZ,IPC_CREAT|0666|IPC_EXCL)) < 0) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }
    if((shm = shmat(shmid,NULL,0))==(char *) -1){
        perror("shmat");
        exit(EXIT_FAILURE);
    }*/

    shm = attach_memory_block(SHMFILENAME, SHMSZ);

    if (shm == NULL) exit(EXIT_FAILURE);

    printf("Aquí no\n");

    kill(getppid(), SIGUSR1);

    s = shm;
    for(c='a'; c <= 'z'; c++) *s++ = c;
    //while (*shm != '*') sleep(1);

    dettach_memory_block(shm);

    exit(EXIT_SUCCESS);
}