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
int Finalizar_ejecucion = 0;
int usr1_arrived = 0, usr2_arrived = 0;
int im_candidate = 0;
sem_t *mutex, *file_mutex, *candidate, *n_votantes;
