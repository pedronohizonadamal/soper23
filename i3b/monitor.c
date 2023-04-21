#include "monitor.h"
#include "pow.h"

/* NOTA: LOS SEMÁFOROS DEBEN SER SIN NOMBRE Y ESTAR DENTRO DEL SEGMENTO DE MEMORIA COMPARTIDA*/
int main(int argc, char **argv)
{
    bool comprobador = true;
    int shm_mem;
    int lag;
    int queue_pos = 0;
    int completion_flag = 0;
    struct MemoryQueue *mem_queue = NULL;
    struct Block block;
    mqd_t queue;
    pid_t pid;

    if (argc != 2)
    {
        printf("Incorrect arguments. Usage: ./monitor <lag>\n");
        exit(EXIT_FAILURE);
    }
    lag = atoi(argv[1]);

    /*Init shared memory*/
    shm_mem = shm_open(SHARED_MEMORY_NAME, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
    if (shm_mem == -1)
    {
        /*The memory was created, become monitor and open the file*/
        comprobador = false;
        shm_mem = shm_open(SHARED_MEMORY_NAME, O_RDWR, S_IRUSR | S_IWUSR);
    }

    if (ftruncate(shm_mem, sizeof(struct MemoryQueue)) == -1)
    {
        perror("ftruncate");
        shm_unlink(SHARED_MEMORY_NAME);
        close(shm_mem);
        exit(EXIT_FAILURE);
    }

    mem_queue = mmap(NULL, sizeof(struct MemoryQueue), PROT_READ | PROT_WRITE, MAP_SHARED, shm_mem, 0);
    close(shm_mem);
    if (mem_queue == MAP_FAILED)
    {
        perror("mmap");
        shm_unlink(SHARED_MEMORY_NAME);
        exit(EXIT_FAILURE);
    }

    if (init_semaphores(&mem_queue->mutex, &mem_queue->queue_space, &mem_queue->queue_blocks) == false)
        //Probar con y sin unmap
        exit(EXIT_FAILURE);

    pid = fork();

    if(pid < 0){
        printf("Fork error\n");
        munmap(mem_queue, sizeof(struct MemoryQueue));
        sem_destroy(&mem_queue->mutex);
        sem_destroy(&mem_queue->queue_space);
        sem_destroy(&mem_queue->queue_blocks);
        shm_unlink(SHARED_MEMORY_NAME);
        exit(EXIT_FAILURE);
    } else if (pid == 0){
        // Proceso monitor
        printf("[%d] Printing blocks...\n", (int)getpid());
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
            {
                if (block.flag == true)
                {
                    printf("Solution accepted: %08ld --> %08ld\n", block.target, block.solution);
                }
                else
                {
                    printf("Solution rejected: %08ld !-> %08ld\n", block.target, block.solution);
                }
                // Wait
                sleep(lag);
            }
        }
    }
    else
    {
        // Proceso comprobador
        /*init_semaphores(mutex, queue_space, queue_blocks);*/
        printf("[%d] Checking blocks...\n", (int)getpid());

        /* Message Queue */
        queue = mq_open(MQ_NAME, O_RDONLY, S_IRUSR | S_IWUSR, &attributes);

        if (queue == (mqd_t)-1)
        {
            perror("Queue error");
            munmap(mem_queue, sizeof(struct MemoryQueue));
            sem_destroy(&mem_queue->mutex);
            sem_destroy(&mem_queue->queue_space);
            sem_destroy(&mem_queue->queue_blocks);
            shm_unlink(SHARED_MEMORY_NAME);
            exit(EXIT_FAILURE);
        }

        // Loop until we receive the special completion block
        while (!completion_flag)
        {
            // Receive a block
            if (!(receive_block(&block, queue)))
            {
                completion_flag = 1;
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

            // Insert the block into shared memory
            sem_wait(&mem_queue->queue_space);
            sem_wait(&mem_queue->mutex);
            mem_queue->queue[queue_pos % QUEUE_SIZE] = block;
            sem_post(&mem_queue->mutex);
            sem_post(&mem_queue->queue_blocks);
            queue_pos++;

            // Wait. Wait isn't needed if about to end
            if(!completion_flag)
                sleep(lag);
        }

        sem_destroy(&mem_queue->mutex);
        sem_destroy(&mem_queue->queue_space);
        sem_destroy(&mem_queue->queue_blocks);
    }
    // Free resources and terminate
    printf("[%d] Finishing\n", (int)getpid());
    munmap(mem_queue, sizeof(struct MemoryQueue));
    shm_unlink(SHARED_MEMORY_NAME);
    mq_close(queue);
    mq_unlink(MQ_NAME);
    wait(NULL);
    exit(EXIT_SUCCESS);
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
        perror("message error");
        return -1;
    }
    return 1;
}

bool init_semaphores(sem_t *mutex, sem_t *space, sem_t *blocks)
{
    if (sem_init(mutex, 1, 1) == -1) {
        perror("sem_init");
        shm_unlink(SHARED_MEMORY_NAME);
        return false;
    }


    if (sem_init(space, 1, QUEUE_SIZE) == -1) {
        perror("sem_init");
        sem_destroy(mutex);
        shm_unlink(SHARED_MEMORY_NAME);
        return false;
    }


    if (sem_init(blocks, 1, 0) == -1) {
        perror("sem_init");
        sem_destroy(mutex);
        sem_destroy(space);
        shm_unlink(SHARED_MEMORY_NAME);
        return false;
    }

    return true;
}
