#include "votante.h"


void sigint_handler(int sig) {
    pid_t wpid;
    int status;

    kill(-1*gpid,SIGTERM);
    while ((wpid = wait(&status)) > 0);
    printf("Finishing by signal\n");
    exit(EXIT_SUCCESS);
}

void init_semaphores(){
  sem_init(&mutex, 1, 1);
  return;
}

void close_semaphores(){

}
int main(int argc, char **argv)
{
  pid_t pid, wpid;
  int status;
  int processes;
  int seconds;
  int i;
  FILE *f=NULL;
  struct sigaction act;


  if (argc != 3)
  {
    printf("Not enough arguments passed to principal\nUsage: <processes> <seconds>\n");
    return -1;
  }
  processes = atoi(argv[1]);
  seconds = atoi(argv[2]);
  
  if (!(f = fopen("Register", "w"))){
    exit(EXIT_FAILURE);
  }

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
      execvp("./votante", NULL);
      perror("execvp");
      exit(EXIT_FAILURE);
    }

    else {
      if(gpid == -1){
        gpid = pid;  /*Set the group process id to the first child's id*/
      }
      setpgid(pid, gpid);
      /*Print in register*/
      fprintf(f, "%d| ", pid);
      
    }
  }
  
  /*Link sigint to sigint_handler*/
  act.sa_handler = sigint_handler;
  sigemptyset(&act.sa_mask);
  act.sa_flags= 0;
  /*Error control*/
  if(sigaction(SIGINT, &act, NULL) < 0){
      perror("sigaction error");
      exit(EXIT_FAILURE);
  }
  /*Start voters*/
  sleep(1); /*Sustituir*/
  kill(-1*gpid,SIGUSR1);

  sleep(seconds);
  /*Send sig to every process in the group*/
  kill(-1*gpid,SIGTERM);
  while ((wpid = wait(&status)) > 0);
  printf("Finishing by alarm");  /*SUS NO USAMOS UNA ALARMA*/
  exit(EXIT_SUCCESS);
}
