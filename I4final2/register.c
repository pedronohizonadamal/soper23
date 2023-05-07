#include "register.h"

// Funcion que implementa el proceso registrador
int registrador(int *pipe_) {

    struct Block block;
    char filename[FILENAME_SIZE];
    FILE *fp = NULL;

    // Blocks sigint and sigalarm
    resgister_startup();
    
    close(pipe_[1]);

    snprintf(filename, sizeof(filename), "log_%d", getppid());

    if ((fp = fopen(filename, "w")) == NULL ) {
        perror("file error");
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

void resgister_startup() {
    sigset_t  sig_int_alarm;
    sigemptyset(&sig_int_alarm);
    sigaddset(&sig_int_alarm, SIGINT);
    sigaddset(&sig_int_alarm, SIGALRM);
    //No queremos recibir SIGINT ni SIGALARM hasta el final de cada ronda
    sigprocmask(SIG_BLOCK,&sig_int_alarm,NULL);
}



