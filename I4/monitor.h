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
#include <stdbool.h>
#include <semaphore.h>
#include <mqueue.h>
#include <signal.h>

#define SHMSZ 27
#define SHMFILENAME "shmfile.h"

#define IPC_RESULT_ERROR -1
#define SHARED_MEMORY_NAME "monitor_shmem"
#define SEMAPHORE_MUTEX "candidate_mutex"
#define SEMAPHORE_MULTIPLEX_IN "candidate_multiplex_in"
#define SEMAPHORE_MULTIPLEX_OUT "candidate_multiplex_out"
#define MINER_CANDIDATE_QUEUE_IN "miner_candidate_multiplex_in"
#define MINER_CANDIDATE_QUEUE_OUT "miner_candidate_multiplex_out"
#define COMPLETION_BLOCK "XD"
#define INITAL_ANSWER 0

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
    int n_mineros;
    int n_votos;
    int n_votos_pos;
    bool initialised;
    bool flag;
    bool end;
};

struct MemoryQueue {
    struct Block queue[QUEUE_SIZE];
    sem_t mutex;
    sem_t queue_space;
    sem_t queue_blocks;
};

struct mq_attr attributes = {. mq_flags = 0 ,
    . mq_maxmsg = 7 ,
    . mq_curmsgs = 0 ,
    . mq_msgsize = sizeof ( struct Block ) };

int receive_block(struct Block *block, mqd_t queue);
void check_block(struct Block *block);
long get_sol(long target);
bool init_semaphores(sem_t *mutex, sem_t *space, sem_t *blocks);

#endif
