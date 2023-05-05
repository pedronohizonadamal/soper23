#include "pow.h"
#include "miner.h"
#include "register.h"

sem_t *mutex, *ganador;
struct Network *network;
sigset_t sig_usr1, sig_usr2, sig_int_alarm, oldmask;
bool first_miner;
int n_threads;
long solution = -1;
int network_id;
int usr1_arrived = 0;
int usr2_arrived = 0;
int end = 0;

void sigusr1_handler(int sig){
    usr1_arrived = 1;
}

void sigusr2_handler(int sig){
    usr2_arrived = 1;
    solution = 0; //Esto causará que los hilos de minado terminen
}

void sigint_sigalarm_handler(int sig){
    end = 1;
}

int main (int argc, char **argv) {
    int n_seconds;
    int intentos;
    int mi_voto;  
    long target;
    int pipe_to_registrador[2];
    pid_t pid;

    if (argc != 3) {
        printf("Incorrect arguments. Usage: ./miner <N_SECONDS> <N_THREADS>\n");
        exit(EXIT_FAILURE);
    }
 
    n_seconds = atoi(argv[1]);
    if(n_seconds <= 0)
        exit(EXIT_SUCCESS);
    n_threads = atoi(argv[2]);

    //TO DO: Crear e implementar registrador
    if (init_pipe(pipe_to_registrador) == false) exit(EXIT_FAILURE);

    pid = fork();

    if (pid == -1) exit(EXIT_FAILURE);

    // Proceso Registrador
    if (pid == 0) registrador(pipe_to_registrador);

    // Proceso Minero
    else miner_rush(n_seconds, intentos, mi_voto, target, pipe_to_registrador);
    

    exit(EXIT_SUCCESS);
}

//Todo el proceso de conexión a memoria y registro del minero está encapsulado aquí
void miner_startup(){
    int shm_mem;
    struct sigaction act;

    if((mutex = sem_open(MUTEX, O_CREAT, S_IRUSR | S_IWUSR ,1))==SEM_FAILED){
        perror("mutex semaphore error");
        exit(EXIT_FAILURE);
    }

    if((ganador = sem_open(GANADOR, O_CREAT, S_IRUSR | S_IWUSR ,1))==SEM_FAILED){
        perror("ganador semaphore error");
        sem_close(mutex);
        exit(EXIT_FAILURE);
    }

    sem_wait(mutex);
    /* Creación o apertura del segmento de memoria compartida */
    shm_mem = shm_open(MINER_NETWORK, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
    if (shm_mem == -1)
    {
        /*The memory had been created*/
        first_miner = false;
        shm_mem = shm_open(MINER_NETWORK, O_RDWR, S_IRUSR | S_IWUSR);
    } else {
        //We are the first ones
        first_miner = true;
        //truncate file
        if (ftruncate(shm_mem, sizeof(struct Network)) == -1) {
            perror("ftruncate");
            shm_unlink(MINER_NETWORK);
            sem_unlink(MUTEX);
            sem_unlink(GANADOR);
            close(shm_mem);
            exit(EXIT_FAILURE);
        }

    }

    //Map the shared memory file to the network variable
    network = mmap(NULL, sizeof(struct Network), PROT_READ | PROT_WRITE, MAP_SHARED, shm_mem, 0);
    close(shm_mem);
    if (network == MAP_FAILED)
    {
        perror("mmap");
        if(first_miner == true){
            shm_unlink(MINER_NETWORK);
            sem_unlink(MUTEX);
            sem_unlink(GANADOR);
            exit(EXIT_FAILURE);
        } else {
            sem_post(mutex);
            sem_close(mutex);
            sem_close(ganador);
            exit(EXIT_FAILURE);
        }
        
    }
    if(first_miner == true){
        init_network(network);
    }
        
    //El minero registra su pid en el sistema
    if (!registrar_pid(network)){
        munmap(network,sizeof(struct Network));
        sem_post(mutex);
        sem_close(ganador);
        sem_close(mutex);
        exit(EXIT_FAILURE);
    }

    sem_post(mutex);

    //Asignar handlers
    sigemptyset(&act.sa_mask);
    act.sa_flags= 0;
    act.sa_handler = sigusr1_handler;
    if(sigaction(SIGUSR1, &act, NULL) < 0){
        perror("sigaction error");
        exit(EXIT_FAILURE);
    }
    act.sa_handler = sigusr2_handler;
    if(sigaction(SIGUSR2, &act, NULL) < 0){
        perror("sigaction error");
        exit(EXIT_FAILURE);
    }
    act.sa_handler = sigint_sigalarm_handler;
    if(sigaction(SIGALRM, &act, NULL) < 0){
        perror("sigaction error");
        exit(EXIT_FAILURE);
    }
    if(sigaction(SIGINT, &act, NULL) < 0){
        perror("sigaction error");
        exit(EXIT_FAILURE);
    }
    sigfillset(&sig_usr1);
    sigdelset(&sig_usr1, SIGUSR1);
    sigdelset(&sig_usr1, SIGINT);
    sigdelset(&sig_usr1, SIGALRM);
    sigfillset(&sig_usr2);
    sigdelset(&sig_usr2, SIGUSR2);
    sigdelset(&sig_usr2, SIGINT);
    sigdelset(&sig_usr2, SIGALRM);
    sigemptyset(&sig_int_alarm);
    sigaddset(&sig_int_alarm, SIGINT);
    sigaddset(&sig_int_alarm, SIGALRM);
    return;
}

void close_minero(int *pipe_){
    int mineros_activos = 0;
    int i;
    int status;
    struct Block finish;
    sem_wait(mutex);
    for(i = 0; i<MAX_MINEROS; i++){
        if(network->pids[i] != 0){
            mineros_activos++;
        }
    }
    if(mineros_activos > 1){ //No soy el último minero, desligar y terminar
        network->pids[network_id] = 0;
        network->monederos[network_id].pid = 0;
        network->monederos[network_id].monedas = 0;
        munmap(&network,sizeof(struct Network));
        sem_post(mutex);
        sem_close(ganador);
        sem_close(mutex);
        exit(EXIT_SUCCESS);
    } else { //Soy el último minero
        init_block(&finish);
        finish.end = true;
        send_block(&finish); //enviar bloque de finalización si el monitor existe
        shm_unlink(MINER_NETWORK);
        sem_close(ganador);
        sem_unlink(GANADOR);
        munmap(&network,sizeof(struct Network));
        sem_post(mutex);
        sem_close(mutex);
        sem_unlink(MUTEX);
        /*TO DO:"espera la finalización de su proceso Registrador, quien debe detectar el cierre de la
tuber ́ıa, e imprime un mensaje de aviso en el caso de que no termine con el c ́odigo de salida
EXIT_SUCCESS"*/
        close(pipe_[1]);
        wait(&status);

        if(WEXITSTATUS(status) != EXIT_SUCCESS){
        printf("Warning: registrador exited without EXIT_SUCCESS\n");
        }
        
        exit(EXIT_SUCCESS);
    }
}

void *search(void *s){
    long int i;
    int not_found = -1;
    struct Search_space search = *(struct Search_space*)s;
    
    for(i = search.lower; i<search.upper && solution==-1; i++){
        if (pow_hash(i) == search.target){
            solution = i;
            pthread_exit((void *) &i);
        }
    }
    pthread_exit((void *) &not_found);
} 

void get_sol(long target){
    pthread_t *thread;
    long search_area;
    struct Search_space *s;
    int i;

    search_area = (long) ceil(((float) POW_LIMIT)/n_threads);

    
    /*allocate memory*/
    if (!(thread = (pthread_t *) malloc(sizeof(pthread_t)*n_threads))){
        printf("Error allocating memory");
        return;
    }
    if (!(s = (struct Search_space *) malloc(sizeof(struct Search_space)*n_threads))){
        free(thread);
        printf("Error allocating memory");
        return;
    }

    /*Crear hilos*/
    for(i = 0; i<n_threads; i++){
        s[i].lower = search_area*i;
        s[i].upper = search_area*(i+1);
        s[i].target = target;
        if (pthread_create(&thread[i], NULL, search, &s[i]) != 0){
            free(s);
            free(thread);
            exit(EXIT_FAILURE);
        }
    }
    /*Esperar hilos*/
    for(i = 0; i<n_threads; i++){
        if (pthread_join(thread[i], NULL)!=0){
            free(s);
            free(thread);
            exit(EXIT_FAILURE);
        }
    }
    /*Liberar memoria*/
    free(thread); 
    free(s);
    return;
}

void init_block (struct Block *block) {
    int i;
    block->block_id = 0;
    block->target = 0;
    block->solution = 0;
    block->n_votos = 0;
    block->pid_ganador = 0;
    for(i = 0; i<MAX_MINEROS; i++){
        block->monederos[i].monedas = 0;
        block->monederos[i].pid = 0;
    }
    block->n_votos_pos = 0;
    block->flag = false;
    block->end = false;
}

//This function copies current_block to last_block and then creates a new one in current_block
void new_round_block (struct Block *current_block, struct Block *last_block) {
    int i;

    // id
    last_block->block_id = current_block->block_id;
    current_block->block_id++;

    // end
    last_block->end = current_block->end;
    current_block->end = false;

    // monederos
    for(i = 0; i<MAX_MINEROS; i++){
        last_block->monederos[i] = current_block->monederos[i];
    }

    // votos
    last_block->n_votos = current_block->n_votos;
    current_block->n_votos = 0;
    
    // votos positivos
    last_block->n_votos_pos = current_block->n_votos_pos;
    current_block->n_votos_pos = 0;

    // ganador
    last_block->pid_ganador = current_block->pid_ganador;
    current_block->pid_ganador = 0;

    // solucion
    last_block->solution = current_block->solution;
    last_block->target = current_block->target;

    //target
    current_block->target = current_block->solution;
    current_block->solution = -1;
    return;
}

void init_network (struct Network *net){
    int i;
    for(i = 0; i<MAX_MINEROS; i++){
        net->pids[i] = 0;
        net->votos[i] = 0;
        net->monederos[i].pid = 0;
        net->monederos[i].monedas = 0;
    }
    init_block(&net->current_block);
    init_block(&net->last_block);
    return;
}

int registrar_pid(struct Network *net){
    int i;
    for(i = 0; i<MAX_MINEROS; i++){
        if(net->pids[i] == 0){
            net->pids[i] = getpid();
            net->monederos[i].monedas = 0;
            net->monederos[i].pid = getpid();
            network_id = i;
            return 1;
        }
    }
    //El sistema está lleno
    return 0;
}

void inicializar_votos(struct Network *net){
    int i;
    for(i = 0; i<MAX_MINEROS; i++){
        net->votos[i] = 0;
    }
    return;
}

//Función que gestiona la votación desde el punto de vista del ganador
int check_votes(){
    int votos_totales=0;
    int votos_positivos=0;
    int mineros_sin_votar = 0;
    int i;
    sem_wait(mutex);
    for(i = 0; i<MAX_MINEROS; i++){
        //Por cada minero activo
        if(network->pids[i] != 0){
            if(network->votos[i] == 0){
                //Hay un minero sin votar
                mineros_sin_votar++;
            } else {
                votos_totales++;
                if(network->votos[i] == 1)
                    votos_positivos++;
            }
        }
    }

    network->current_block.n_votos = votos_totales;
    network->current_block.n_votos_pos = votos_positivos;

    sem_post(mutex);

    if(mineros_sin_votar == 1){ //Hay un único minero sin votar, somos nosotros
        return 1; //Terminar la votación
    }

    return 0; //No ha terminado la votación
}

void send_block(struct Block *block){
    mqd_t queue;
    struct timespec time = {1, 500};
    // Crear cola de mensajes
    queue = mq_open(MQ_NAME, O_WRONLY, S_IRUSR | S_IWUSR, &attributes);

    if (queue == (mqd_t)-1)
    {
        //There is no monitor or we couldn't access it, we won't send the message
        mq_close(queue);
        return;
    }

    if (mq_timedsend(queue, (char *)block, sizeof(struct Block), 1, &time) == -1)
    {
        perror("Send error");
        mq_close(queue);
        exit(EXIT_FAILURE);
    }
    mq_close(queue);
    return;
}

void send_signals(int signal){
    int i;
    for(i = 0; i<MAX_MINEROS; i++){
        if(network->pids[i] != 0 && i!=network_id){
            kill(network->pids[i], signal);
        }
    }
    return;
}
//TO DO: modularizar todas las funciones (separarlas en diferentes .c en función de su categoría)

// Función que implementa el proceso minero
void miner_rush (int n_seconds, int intentos, int mi_voto, long target, int *pipe_) {
    miner_startup();
    close(pipe_[0]);
    //Setup alarm
    alarm(n_seconds);
    //No queremos recibir SIGINT ni SIGALARM hasta el final de cada ronda
    sigprocmask(SIG_BLOCK,&sig_int_alarm,NULL);

    if(first_miner == true){
        //Enviar SIGUSR1
        send_signals(SIGUSR1);
    } else {
        //Esperar comienzo de ronda
        sigprocmask(SIG_BLOCK,&sig_usr1,&oldmask);
            while(usr1_arrived == 0){
                sigsuspend(&oldmask);
            }
            usr1_arrived = 0;
        sigprocmask(SIG_UNBLOCK,&sig_usr1,NULL);
    }
    while(!end){
        sem_wait(mutex);
        target = network->current_block.target;
        sem_post(mutex);

        solution = -1;
        get_sol(target);

        if(!(sem_trywait(ganador))){
            //Proceso ganador
            
            //Preparar votación
            sem_wait(mutex);
            network->current_block.solution = solution;
            network->current_block.pid_ganador = getpid();
            inicializar_votos(network);
            sem_post(mutex);

            //enviar USR2
            send_signals(SIGUSR2);
            
            //Comprobar votación
            intentos = 0;
            while(intentos < MAX_INTENTOS){
                sleep(1);
                if(check_votes())
                    break;
                intentos++;
            }

            sem_wait(mutex);
            if(network->current_block.n_votos_pos*2 >= network->current_block.n_votos){
                network->monederos[network_id].monedas++; //Si hemos ganado, sumar una moneda
            }
            //Copiar monederos al bloque actual
            for(int i=0;i<MAX_MINEROS; i++){
                network->current_block.monederos[i] = network->monederos[i];
            }
            //Enviar bloque
            send_block(&network->current_block);
            //Preparar siguiente ronda
            new_round_block(&network->current_block,&network->last_block);
            sem_post(mutex);
            sem_post(ganador);  

            //Enviar Usr1
            send_signals(SIGUSR1);

            //TO DO:Enviar al Registrador por pipe
            sem_wait(mutex);
            write(pipe_[1], &(network->last_block), sizeof(struct Block));
            sem_post(mutex);


        } else {
            //Proceso perdedor

            //Esperar USR2
            sigprocmask(SIG_BLOCK,&sig_usr2,&oldmask);
                while(usr2_arrived == 0){
                    sigsuspend(&oldmask);
                }
                usr2_arrived = 0;
            sigprocmask(SIG_UNBLOCK,&sig_usr2,NULL);

            //Votar
            sem_wait(mutex);
            if (pow_hash(network->current_block.solution) == network->current_block.target){
                mi_voto = 1;
            } else {
                mi_voto = -1;
            }
            network->votos[network_id] = mi_voto;
            sem_post(mutex);

            //Esperar USR1
            sigprocmask(SIG_BLOCK,&sig_usr1,&oldmask);
                while(usr1_arrived == 0){
                    sigsuspend(&oldmask);
                }
                usr1_arrived = 0;
            sigprocmask(SIG_UNBLOCK,&sig_usr1,NULL);

            //TO DO: Enviar al Registrador por pipe
            sem_wait(mutex);
            write(pipe_[1], &(network->last_block), sizeof(struct Block));
            sem_post(mutex);
        }

        //Comprobar si ha llegado la alarma o SIGINT
        sigprocmask(SIG_UNBLOCK,&sig_int_alarm,NULL);
        if(end){
            close_minero(pipe_);
        }
        sigprocmask(SIG_BLOCK,&sig_int_alarm,NULL);
    }
    
    close_minero(pipe_);
}
