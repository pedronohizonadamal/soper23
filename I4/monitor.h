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
#include <sys/time.h>
#include <pthread.h>
#include <math.h>

#define SHMSZ 27
#define SHMFILENAME "shmfile.h"

#define IPC_RESULT_ERROR -1
#define MINER_NETWORK "miner_network"
#define SEMAPHORE_MUTEX "candidate_mutex"
#define SEMAPHORE_MULTIPLEX_IN "candidate_multiplex_in"
#define SEMAPHORE_MULTIPLEX_OUT "candidate_multiplex_out"
#define MINER_CANDIDATE_QUEUE_IN "miner_candidate_multiplex_in"
#define MINER_CANDIDATE_QUEUE_OUT "miner_candidate_multiplex_out"
#define MONITOR_COMPROBADOR_SHARED_MEMORY "monitor_shmem"
#define COMPLETION_BLOCK "XD"
#define INITIAL_TARGET 0
#define MUTEX "voting_shared_mutex"
#define GANADOR "voting_ganador_perdedor"
#define MAX_INTENTOS 2

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

struct Network {
    pid_t pids[MAX_MINEROS];
    int votos[MAX_MINEROS];
    struct Monedero monederos[MAX_MINEROS];
    struct Block last_block;
    struct Block current_block;
};

struct Search_space
{
  long int lower;
  long int upper;
  long int target;

};

struct mq_attr attributes = {. mq_flags = 0 ,
    . mq_maxmsg = 7 ,
    . mq_curmsgs = 0 ,
    . mq_msgsize = sizeof ( struct Block ) };

int receive_block(struct Block *block, mqd_t queue);
void check_block(struct Block *block);
void get_sol(long target);
bool init_semaphores(sem_t *mutex, sem_t *space, sem_t *blocks);
void init_network (struct Network *net);
int registrar_pid(struct Network *net);
void miner_startup();
void inicializar_votos(struct Network *net);
int check_votes();
void send_block(struct Block *block);
void init_block (struct Block *block);
void new_round_block (struct Block *current_block, struct Block *last_block);
void close_minero();
void send_signals(int signal);

#endif
