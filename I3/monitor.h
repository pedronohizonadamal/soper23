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
#include <unistd.h> 
#include <sys/wait.h>

#define SHMSZ 27
#define SHMFILENAME "shmfile.h"

#define IPC_RESULT_ERROR -1
#define SHARED_MEMORY_NAME "monitor_shmem"
#define COMPLETION_BLOCK "XD"

struct Memory{
    int block;
};

static int get_shared_block(char *filename, int size);

char *attach_memory_block(char *filename, int size);

bool dettach_memory_block(char *block);

bool destroy_memory_block(char *filename);

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




#endif



