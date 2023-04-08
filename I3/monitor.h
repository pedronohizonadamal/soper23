#ifndef MONITOR_H
#define MONITOR_H

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h> 
#include <sys/mman.h>
#include <signal.h>
#include <unistd.h> 
#include <sys/wait.h>
#include <stdbool.h>

#define SHMSZ 27
#define SHMFILENAME "shmfile.h"

#define IPC_RESULT_ERROR -1

static int get_shared_block(char *filename, int size);

char *attach_memory_block(char *filename, int size);

bool dettach_memory_block(char *block);

bool destroy_memory_block(char *filename);

#endif



