#ifndef MINER_RUSH_H
#define MINER_RUSH_H

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
#include <stdbool.h>
#include <semaphore.h>
#include <mqueue.h>
#include <signal.h>
#include <sys/time.h>
#include <pthread.h>
#include <math.h>
#include <string.h>

#define QUEUE_SIZE 5

#define MQ_NAME "/m_queue"

#define MAX_MINEROS 100

struct Monedero {

    int monedas;
    pid_t pid;
};

struct Block{

    int block_id;
    long int target;
    long int solution;
    pid_t pid_ganador;
    struct Monedero monederos[MAX_MINEROS];
    int n_votos;
    int n_votos_pos;
    bool flag;
    bool end;
};

struct MemoryQueue {
    struct Block queue[QUEUE_SIZE];
    sem_t mutex;
    sem_t queue_space;
    sem_t queue_blocks;
};
// Imprime un blocque a fichero
void print_block (FILE *fp, struct Block block);

#endif