#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>

#define REGISTER "Register.txt"
#define LINE_BUFFER 256
#define WAITING_TIME {0,1000}
#define WAITING_ROUND {0, 250000}
#define WAITING_AUX {0, 20000}
#define RESULTS_SIZE 4096

void start_round();

int gpid = -1;
int processes;
int Finalizar_ejecucion = 0;
int Allow_term = 0;
int usr1_arrived = 0, usr2_arrived = 0;
sem_t *mutex, *file_mutex, *candidate, *n_votantes; 
char *results;
