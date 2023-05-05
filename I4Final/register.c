#include "register.h"

// Funcion que implementa el proceso registrador
int registrador(int *pipe_) {

    struct Block block;
    char filename[FILENAME_SIZE];
    FILE *fp = NULL;
    
    close(pipe_[1]);

    snprintf(filename, sizeof(filename), "log_%d", getppid());

    if ((fp = fopen(filename, "w")) == NULL ) {
        perror("CRITICAL ERROR OPENING FILE\n");
        return EXIT_FAILURE;
    }

    while (read(pipe_[0],&block ,sizeof(struct Block)) != 0) {
    
        block.flag = (2 * block.n_votos_pos) >= block.n_votos;

        print_block(fp, block);

        if (block.end == true) break;
    }

    fclose(fp);
    close(pipe_[0]);

    return EXIT_SUCCESS;
}

//Función que iniciliza el pipe
bool init_pipe (int *pipe_) {
    if(pipe(pipe_) == -1) {
        perror("Pipe error\n");
        return false;
    }
    return true;
}


