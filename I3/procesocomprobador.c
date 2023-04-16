#include "monitor.h"


int main(int argc, char **argv){
    pid_t pid;
    int shm_mem;
    int lag;
    int completion_flag = 0;
    long block;
    Memory *mem = NULL;
    
    if(argc != 2){
        printf("Incorrect arguments. Usage: ./monitor <lag>\n");
        exit(EXIT_FAILURE);
    }
    lag = atoi(argv[1]);
    shm_mem = shm_open(SHARED_MEMORY_NAME, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
    if(shm_mem == -1){
        perror("The memory was already created");
        exit(EXIT_FAILURE);
    }
    
    pid = fork();

    if(pid <0){
        perror("Fork error");
        exit(EXIT_FAILURE);
    } else if(pid > 0){
        execvp("./procesomonitor", &argv[1]);
        perror("execvp");
        exit(EXIT_FAILURE);
    } else {
        // Proceso candidato
        if (ftruncate(shm_mem, sizeof(Memory)) == -1) {
            perror("ftruncate");
            shm_unlink(SHARED_MEMORY_NAME);
            close(shm_mem);
            exit(1);
        }
        mem = mmap(NULL, sizeof(Memory), PROT_READ | PROT_WRITE, MAP_SHARED, shm_mem, 0);
        close(shm_mem);
        if (mem == MAP_FAILED) {
            perror("mmap");
            shm_unlink(SHARED_MEMORY_NAME);
            exit(1);
        }

        // Loop until we receive the special completion block
        while (!completion_flag) {
            // Receive a block
            receive_block(&block);
            check_block(block);

            // Check if the received block is the special completion block
            if(block == COMPLETION_BLOCK){
                completion_flag = 1;
            }

            // Insert the block into shared memory
            mem->block = block;

            // Wait
            sleep(lag);
        }

        // Insert the completion block into shared memory
        mem->block = block;

        // Free resources and terminate
        munmap(mem, sizeof(Memory));
        shm_unlink(SHARED_MEMORY_NAME);
        wait(NULL);
        exit(EXIT_SUCCESS);
        }
}