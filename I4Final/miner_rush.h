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

struct mq_attr attributes = {. mq_flags = 0 ,
    . mq_maxmsg = 7 ,
    . mq_curmsgs = 0 ,
    . mq_msgsize = sizeof ( struct Block ) };
 
void print_block (FILE *fp, struct Block block); 

// Imprime un blocque a fichero
void print_block (FILE *fp, struct Block block) {

    int i;

    fprintf(fp, "Id:       %04d\n",block.block_id);
    fprintf(fp, "Winner:   %d\n",block.pid_ganador);
    fprintf(fp, "Target:   %08ld\n",block.target);
    if (block.flag == true)
    {
        fprintf(fp, "Solution: %08ld (validated)\n", block.solution);
    }
    else
    {
        fprintf(fp, "Solution: %08ld (invalidated)\n", block.solution);
    }
    fprintf(fp, "Votes:    %d/%d\n",block.n_votos_pos,block.n_votos);
    fprintf(fp, "Wallets:");
    for(i = 0; i<MAX_MINEROS; i++){
        if(block.monederos[i].pid != 0){
            fprintf(fp, "  %d:%02d",block.monederos[i].pid,block.monederos[i].monedas);
        }
    }
    fprintf(fp, "\n\n");
}

#endif