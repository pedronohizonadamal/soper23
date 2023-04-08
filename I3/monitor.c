#include "monitor.h"

int main (int argc, char **argv) {
  pid_t pid;
  int lag;
  FILE *f;
  int shmid;
  sigset_t usr1;
  int status;
  char *shm=NULL, *s=NULL;
  
  if (argc != 2) {
      printf("Not enough arguments passed to monitor\nUsage: ./monitor <lag>\n");
      exit(EXIT_FAILURE);
  }

  lag = atoi(argv[1]);
  

  /* Fork */

  pid = fork();

  if (pid < 0)
  {
    perror("fork");
    exit(EXIT_FAILURE);
  }

  else if (pid == 0)
  {
    execvp("./comprobador", &argv[1]);
    perror("execvp");
    exit(EXIT_FAILURE);
  }

  sigemptyset(&usr1);
  sigaddset(&usr1, SIGUSR1);
  sigprocmask(SIG_BLOCK,&usr1,NULL);


  /* Memoria compartida */

  /*
  if((shmid = shmget(KEY,SHMSZ,0666)) < 0){
    perror("shmget");
    exit(EXIT_FAILURE);
  }

  if ((shm = shmat(shmid,NULL,0))==(char *) -1){
    perror("shmat");
    exit(EXIT_FAILURE);
  }*/

  shm = attach_memory_block(SHMFILENAME, SHMSZ);

  if (shm == NULL) { 
    perror("Fatal shm creation error padre");
    waitpid(pid, &status, 0);
    exit(EXIT_FAILURE);
  }
  
  for(s = shm; *s != NULL; s++) 
    putchar(*s);
  putchar('\n');
  *s = '*';

  dettach_memory_block(shm);
  /*Esperar al hijo*/
  waitpid(pid, &status, 0);

  destroy_memory_block(SHMFILENAME);
  
  exit(status);
}

static int get_shared_block(char *filename, int size) {
    key_t key;

    key = ftok(filename, 0);
    if (key == IPC_RESULT_ERROR) return IPC_RESULT_ERROR;

    return shmget(key, size, 0644 | IPC_CREAT);
}

char *attach_memory_block(char *filename, int size) {
    int shared_block_id = get_shared_block(filename, size);
    char *result;

    if (shared_block_id == IPC_RESULT_ERROR) return NULL;

    result = shmat(shared_block_id, NULL, 0);

    if (result == (char *) IPC_RESULT_ERROR) return NULL;

    return result;
}

bool dettach_memory_block(char *block) {
    return (shmdt(block) != IPC_RESULT_ERROR);
}

bool destroy_memory_block(char *filename) {
    int shared_block_id = get_shared_block(filename, 0);

    if (shared_block_id == IPC_RESULT_ERROR) return false;

    return (shmctl(shared_block_id, IPC_RMID, NULL) != IPC_RESULT_ERROR);
}


