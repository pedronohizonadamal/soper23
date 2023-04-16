
#include "monitor.h"

int main (int argc, char **argv) {
    pid_t pid;
    int shm_mem;
    int lag;
    int completion_flag = 0;
    long block;
    struct MemoryQueue *mem_queue = NULL;
    struct Memory current_mem;
    long old_answer = INITAL_ANSWER;
    
    if(argc != 2){
        printf("Incorrect arguments. Usage: ./monitor <lag>\n");
        exit(EXIT_FAILURE);
    }
    lag = atoi(argv[1]);

    shm_mem = shm_open(SHARED_MEMORY_NAME, O_RDWR , S_IRUSR | S_IWUSR);
    if(shm_mem == -1){
        perror("An error occurred while opening a shared memory segment");
        exit(EXIT_FAILURE);
    }
    
    // Proceso candidato
    /*
    if (ftruncate(shm_mem, sizeof(struct MemoryQueue)) == -1) {
        perror("ftruncate");
        shm_unlink(SHARED_MEMORY_NAME);
        close(shm_mem);
        exit(1);
    }*/

    mem_queue = mmap(NULL, sizeof(struct MemoryQueue), PROT_READ | PROT_WRITE, MAP_SHARED, shm_mem, 0);
    close(shm_mem);
    if (mem_queue == MAP_FAILED) {
        perror("mmap");
        shm_unlink(SHARED_MEMORY_NAME);
        exit(1);
    }

    mem_queue->current_read = 0;

    // Loop until we receive the special completion block
    while (mem_queue->end == false) {

        // Wait to avoid reading empty blocks
        while (mem_queue->current_write <= mem_queue->current_read);

        current_mem = mem_queue->queue[mem_queue->current_read % QUEUE_SIZE];

        print_block(current_mem, old_answer);
        
        old_answer = current_mem.block;

        mem_queue->current_read++;

        // Wait
        sleep(lag);
    }

    // Free resources and terminate
    munmap(mem_queue, sizeof(struct MemoryQueue));
    shm_unlink(SHARED_MEMORY_NAME);
    exit(EXIT_SUCCESS);
        
}

void print_block(struct Memory new_block, long old_answer) {
    if (new_block.flag == false) {
        printf("Solution rejected: %ld !-> %ld\n", old_answer, new_block.block);
    }
    printf("Solution Accepted: %ld --> %ld\n", old_answer, new_block.block);
}
