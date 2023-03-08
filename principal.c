#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char **argv) {
  pid_t pid;
  int status;

  if(argc != 4){
        printf("Not enough arguments passed to principal\n");
        return -1;
  }
  
  pid = fork();
  if (pid < 0) {
    perror("fork");
    exit(EXIT_FAILURE);
  } else if (pid == 0) {
    if (execvp("./minero", &argv[1])) {
      perror("execvp");
      exit(EXIT_FAILURE);
    }
  } else {
    wait(&status);
    printf("Miner exited with status %d\n", WEXITSTATUS(status));
  }
  exit(EXIT_SUCCESS);
}