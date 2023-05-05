#include "comprobador.h"

void insert_block(struct Block *block){
  sem_wait(&mem_queue->queue_space);
  sem_wait(&mem_queue->mutex);
  mem_queue->queue[queue_pos % QUEUE_SIZE] = *block;
  sem_post(&mem_queue->mutex);
  sem_post(&mem_queue->queue_blocks);
  queue_pos++;
}

void check_block(struct Block *block)
{
    if (pow_hash(block->solution) == block->target)
    {
        block->flag = true;
    }
    else
    {
        block->flag = false;
    }
    return;
}

void sigint_handler(int sig) {
  struct Block ending;
  int status;

    //El resto de parámetros nos dan igual
    ending.end = true;
    insert_block(&ending);

    //Free resources
    sem_destroy(&mem_queue->mutex);
    sem_destroy(&mem_queue->queue_space);
    sem_destroy(&mem_queue->queue_blocks);

    // Free resources and terminate
    printf("[%d] Finishing by signal\n", (int)getpid());
    munmap(mem_queue, sizeof(struct MemoryQueue));
    shm_unlink(MONITOR_COMPROBADOR_SHARED_MEMORY);
    mq_close(queue);
    mq_unlink(MQ_NAME);
    //Wait for monitor
    wait(&status);
    if(WEXITSTATUS(status) != EXIT_SUCCESS){
        printf("Warning: monitor exited without EXIT_SUCCESS\n");
    }

exit(EXIT_SUCCESS);
  
}