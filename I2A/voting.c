#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>

int gpid = -1;
sem_t *n_votantes = NULL;

void sigint_handler(int sig) {
    pid_t wpid;
    int status;

    kill(-1*gpid,SIGTERM);
    while ((wpid = wait(&status)) > 0);
    printf("Finishing by signal\n");
    exit(EXIT_SUCCESS);
}

void sigalarm_handler(int sig){
  pid_t wpid;
  int status;
  /*Send sig to every process in the group*/
  kill(-1*gpid,SIGTERM);
  while ((wpid = wait(&status)) > 0);
  printf("Finishing by alarm"); 
  exit(EXIT_SUCCESS);
}

void init_semaphores(){
  if((n_votantes = sem_open("voting_n_votantes", O_CREAT, S_IRUSR | S_IWUSR ,0))==SEM_FAILED){
        perror("N_votantes semaphore error");
        exit(EXIT_FAILURE);
    }
}

void close_semaphores(){
  sem_unlink("voting_n_votantes");
}
int main(int argc, char **argv)
{
  pid_t pid;
  int processes;
  int seconds;
  int i;
  FILE *f=NULL;
  struct sigaction act, alrmhandler;


  if (argc != 3)
  {
    printf("Not enough arguments passed to principal\nUsage: <seconds> <processes>\n");
    return -1;
  }
  processes = atoi(argv[2]);
  seconds = atoi(argv[1]);
  
  if (!(f = fopen("Register", "w"))){
    exit(EXIT_FAILURE);
  }
  init_semaphores();
  /*Spawn children*/
  for (i = 0; i < processes; i++)
  {
    pid = fork();

    if (pid < 0)
    {
      perror("fork");
      exit(EXIT_FAILURE);
    }

    else if (pid == 0)
    {
      execvp("./votante", &argv[2]);
      perror("execvp");
      exit(EXIT_FAILURE);
    }

    else {
      if(gpid == -1){
        gpid = pid;  /*Set the group process id to the first child's id*/
      }
      setpgid(pid, gpid);
      /*Print in register*/
      fprintf(f, "%d\n", pid);
      
    }
  }
  fclose(f);
  
  /*Link sigint to sigint_handler*/
  act.sa_handler = sigint_handler;
  sigemptyset(&act.sa_mask);
  act.sa_flags= 0;
  /*Error control*/
  if(sigaction(SIGINT, &act, NULL) < 0){
      perror("sigaction error");
      exit(EXIT_FAILURE);
  }
  /*Link sigalarm to sigint_handler*/
  alrmhandler.sa_handler = sigalarm_handler;
  sigemptyset(&alrmhandler.sa_mask);
  alrmhandler.sa_flags= 0;
  /*Error control*/
  if(sigaction(SIGALRM, &alrmhandler, NULL) < 0){
      perror("sigaction error");
      exit(EXIT_FAILURE);
  }

  /*Wait until every voter is ready*/
  for(i = 0; i<processes; i++){
    sem_wait(n_votantes);
  }

  /*Send USR1*/
  kill(-1*gpid,SIGUSR1);

  if (alarm(seconds)){
    fprintf(stderr, "There is a previously established alarm\n");
  }

  while(1);
  exit(EXIT_SUCCESS);
}
