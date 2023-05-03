#include "monitor.h"
#include "pow.h"

struct MemoryQueue *mem_queue = NULL;
int queue_pos = 0;
int Interruptions = 0;
mqd_t queue;

void sigint_handler(int sig);
void insert_block(struct Block *block);

int main(int argc, char **argv)
{
    int shm_mem;
    int lag = 1; //Ya no usamos un parámetro variable; Ahora lag es siempre 1
    int completion_flag = 0;
    int i;
    struct Block block;
    pid_t pid;
    int status;
    struct sigaction sigint;
    sigset_t block_signals, oldmask;

    if (argc != 1)
    {
        printf("Incorrect arguments. Usage: ./monitor\n");
        exit(EXIT_FAILURE);
    }

    /*Init shared memory*/
    shm_mem = shm_open(MONITOR_COMPROBADOR_SHARED_MEMORY, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (shm_mem == -1)
    {
        perror("Memory error");
        exit(EXIT_FAILURE);
    }

    //Truncate
    if (ftruncate(shm_mem, sizeof(struct MemoryQueue)) == -1)
    {
        perror("ftruncate");
        shm_unlink(MONITOR_COMPROBADOR_SHARED_MEMORY);
        close(shm_mem);
        exit(EXIT_FAILURE);
    }

    mem_queue = mmap(NULL, sizeof(struct MemoryQueue), PROT_READ | PROT_WRITE, MAP_SHARED, shm_mem, 0);
    close(shm_mem);
    if (mem_queue == MAP_FAILED)
    {
        perror("mmap");
        shm_unlink(MONITOR_COMPROBADOR_SHARED_MEMORY);
        exit(EXIT_FAILURE);
    }

    if (init_semaphores(&mem_queue->mutex, &mem_queue->queue_space, &mem_queue->queue_blocks) == false)
        exit(EXIT_FAILURE);

    pid = fork();

    if(pid < 0){
        printf("Fork error\n");
        munmap(mem_queue, sizeof(struct MemoryQueue));
        sem_destroy(&mem_queue->mutex);
        sem_destroy(&mem_queue->queue_space);
        sem_destroy(&mem_queue->queue_blocks);
        shm_unlink(MONITOR_COMPROBADOR_SHARED_MEMORY);
        exit(EXIT_FAILURE);
    } else if (pid == 0){
        // Proceso monitor

        //Proceso monitor debe blockear todas las señales(excepto KILL y STOP) ya que queremos que finalice por bloque de señalización, y no por un SIGINT dirigido a comprobador
        sigfillset(&block_signals);
        sigprocmask(SIG_BLOCK,&block_signals,NULL);

        // Loop until we receive the special completion block
        while (!completion_flag)
        {
            // Read block from shared memory
            sem_wait(&mem_queue->queue_blocks);
            sem_wait(&mem_queue->mutex);
            block = mem_queue->queue[queue_pos % QUEUE_SIZE];
            sem_post(&mem_queue->mutex);
            sem_post(&mem_queue->queue_space);
            queue_pos++;

            // Check if the received block is the special completion block
            if (block.end == true)
            {
                completion_flag = 1;
            }
            else
            { //Print the block
                printf("Id:       %04d\n",block.block_id);
                printf("Winner:   %d\n",block.pid_ganador);
                printf("Target:   %08ld\n",block.target);
                if (block.flag == true)
                {
                    printf("Solution: %08ld (validated)\n", block.solution);
                }
                else
                {
                    printf("Solution: %08ld (invalidated)\n", block.solution);
                }
                printf("Votes:    %d/%d\n",block.n_votos_pos,block.n_votos);
                printf("Wallets:");
                for(i = 0; i<MAX_MINEROS; i++){
                    if(block.monederos[i].pid != 0){
                        printf("  %d:%02d",block.monederos[i].pid,block.monederos[i].monedas);
                    }
                }
                printf("\n\n");

                // Wait
                sleep(lag);
            }
        }
    }
    else
    {
        // Proceso comprobador

        /* Message Queue */
        queue = mq_open(MQ_NAME, O_RDONLY | O_CREAT, S_IRUSR | S_IWUSR, &attributes);
        if (queue == (mqd_t)-1)
        {
            perror("Queue error");
            munmap(mem_queue, sizeof(struct MemoryQueue));
            sem_destroy(&mem_queue->mutex);
            sem_destroy(&mem_queue->queue_space);
            sem_destroy(&mem_queue->queue_blocks);
            shm_unlink(MONITOR_COMPROBADOR_SHARED_MEMORY);
            exit(EXIT_FAILURE);
        }

        /*Preparar handlers*/
        sigint.sa_handler = sigint_handler;
        sigemptyset(&sigint.sa_mask);
        sigint.sa_flags= 0;
        sigemptyset(&block_signals);
        sigaddset(&block_signals, SIGINT);
        /*Assign handler to usr1*/
        if(sigaction(SIGINT, &sigint, NULL) < 0){
          perror("sigaction error");
          munmap(mem_queue, sizeof(struct MemoryQueue));
          sem_destroy(&mem_queue->mutex);
          sem_destroy(&mem_queue->queue_space);
          sem_destroy(&mem_queue->queue_blocks);
          shm_unlink(MONITOR_COMPROBADOR_SHARED_MEMORY);
          mq_close(queue);
          exit(EXIT_FAILURE);
        }


        // Loop until we receive the special completion block
        while (!completion_flag)
        {
          // Receive a block
          if (!(receive_block(&block, queue)))
          {
              //There was a problem, end
              block.end = true;
          }

          // Check if the received block is the special completion block
          if (block.end == true)
          {
              completion_flag = 1;
          }
          else
          {
              // Comprobar si el bloque es correcto. Añadir al bloque una flag para indicarlo.
              check_block(&block);
          }

          //Block signals during this process
          sigprocmask(SIG_BLOCK,&block_signals,&oldmask);
          // Insert the block into shared memory
          insert_block(&block);
          //Unblock signals
          sigprocmask(SIG_UNBLOCK,&oldmask,NULL);

          // Wait. Wait isn't needed if about to end
          if(!completion_flag)
              sleep(lag);
        }

        sem_destroy(&mem_queue->mutex);
        sem_destroy(&mem_queue->queue_space);
        sem_destroy(&mem_queue->queue_blocks);
    }
    // Free resources and terminate
    munmap(mem_queue, sizeof(struct MemoryQueue));
    shm_unlink(MONITOR_COMPROBADOR_SHARED_MEMORY);
    mq_close(queue);
    mq_unlink(MQ_NAME);
    //Wait for monitor (if we are not monitor)
    if(pid>0){
        wait(&status);
        if(WEXITSTATUS(status) != EXIT_SUCCESS){
        printf("Warning: monitor exited without EXIT_SUCCESS\n");
        }
    }
    
    exit(EXIT_SUCCESS);
}

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

int receive_block(struct Block *block, mqd_t queue)
{
    if (mq_receive(queue, (char *)block, sizeof(*block), NULL) == -1)
    {
        perror("MQ_receive_error");
        return 0;
    }
    return 1;
}

bool init_semaphores(sem_t *mutex, sem_t *space, sem_t *blocks)
{
    if (sem_init(mutex, 1, 1) == -1) {
        perror("sem_init");
        shm_unlink(MONITOR_COMPROBADOR_SHARED_MEMORY);
        return false;
    }


    if (sem_init(space, 1, QUEUE_SIZE) == -1) {
        perror("sem_init");
        sem_destroy(mutex);
        shm_unlink(MONITOR_COMPROBADOR_SHARED_MEMORY);
        return false;
    }


    if (sem_init(blocks, 1, 0) == -1) {
        perror("sem_init");
        sem_destroy(mutex);
        sem_destroy(space);
        shm_unlink(MONITOR_COMPROBADOR_SHARED_MEMORY);
        return false;
    }

    return true;
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