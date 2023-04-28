#include "pow.h"
#include "monitor.h"

void init_block (struct Block *block);

void new_round_block (struct Block *new_block, struct Block old_block);

int main (int argc, char **argv) {

    int n_seconds, n_threads;    
    pid_t pid;
    int shm_mem;
    struct Block block;
    sem_t mutex, init_barrier;
    struct Monedero monedero = { .monedas = 0, .pid = getpid()};
    bool new_shm_mem = true;

    if (argc != 3) {
        printf("Incorrect arguments. Usage: ./miner <N_SECONDS> <N_THREADS>\n");
        exit(EXIT_FAILURE);
    }
 
    n_seconds = atoi(argv[1]);
    n_threads = atoi(argc[2]);

    if((init_barrier = sem_open("voting_shared_mutex", O_CREAT, S_IRUSR | S_IWUSR ,0))==SEM_FAILED){
        perror("init_barrier semaphore error");
        exit(EXIT_FAILURE);
    }

    if((mutex = sem_open("voting_shared_mutex", O_CREAT, S_IRUSR | S_IWUSR ,1))==SEM_FAILED){
        perror("mutex semaphore error");
        exit(EXIT_FAILURE);
    }

    /* Creación segmetno de memoria compartida */
    shm_mem = shm_open(SHARED_MEMORY_NAME, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);

    if (shm_mem == -1)
    {
        /*The memory had been created*/
        new_shm_mem = false;
        shm_mem = shm_open(SHARED_MEMORY_NAME, O_RDWR, S_IRUSR | S_IWUSR);
    }

    if (ftruncate(shm_mem, sizeof(struct Block)) == -1) {

        perror("ftruncate");
        shm_unlink(SHARED_MEMORY_NAME);
        close(shm_mem);
        exit(EXIT_FAILURE);
    }

    block = mmap(NULL, sizeof(struct Block), PROT_READ | PROT_WRITE, MAP_SHARED, shm_mem, 0);
    close(shm_mem);
    if (block == MAP_FAILED)
    {
        perror("mmap");
        shm_unlink(SHARED_MEMORY_NAME);
        exit(EXIT_FAILURE);
    }

    /* Init barrier */
    if (new_shm_mem == false) {
        sem_wait(init_barrier);
        sem_post(init_barrier);
    }

    else {
        init_block(&block);
        sem_post(init_barrier);
    }

    /* Add wallet */
    sem_wait(mutex);
    if (block.n_mineros < MAX_MINEROS) {
        block.monederos[block.n_mineros] = monedero;
        block.n_mineros++;
    }
    else {
        /* make exit safe */
        exit(EXIT_FAILURE);
    }
    sem_post(mutex);

    // pid = fork();

    /* Proceso Registrador */
    if (pid == 0) {

    }



    exit(EXIT_SUCCESS);
}

void init_block (struct Block *block) {
    block->block_id = 0;
    block->target = INITAL_ANSWER;
    block->n_mineros = 0;
    block->n_votos = 0;
    block->n_votos_pos = 0;
    block->initialised = true;
    block->flag = false;
    block->end = false;
}

void new_round_block (struct Block *new_block, struct Block old_block) {
    new_block->block_id = old_block.block_id + 1;
    new_block->target = old_block.solution;
    new_block->n_mineros = 0;
    new_block->n_votos = 0;
    new_block->n_votos_pos = 0;
    new_block->initialised = true;
    new_block->flag = false;
    new_block->end = false;
}