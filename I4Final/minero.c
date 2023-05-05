#include "pow.h"
#include "monitor.h"

sem_t *mutex, *ganador;
struct Network *network;
bool first_miner;
int n_threads;
long solution;
long target;
int network_id;

void init_block (struct Block *block);

void new_round_block (struct Block *new_block, struct Block old_block);

int main (int argc, char **argv) {
    long solution;
    int n_seconds;
    int intentos;
    int mi_voto;
    pid_t pid;
    struct Block block;
    bool Ganador;
    struct Monedero monedero = { .monedas = 0, .pid = getpid()};
    

    if (argc != 3) {
        printf("Incorrect arguments. Usage: ./miner <N_SECONDS> <N_THREADS>\n");
        exit(EXIT_FAILURE);
    }
 
    n_seconds = atoi(argv[1]);
    if(n_seconds <= 0)
        exit(EXIT_SUCCESS);
    n_threads = atoi(argv[2]);

    //TO DO: Crear e implementar registrador

    miner_startup();
    
    //TO DO: Wait/Send Usr1
    while(1){
        //TO DO: Gestionar la llegada correcta de USR2 durante esta función (asignar solucion a 0 para que los hilos terminen)
        solution = -1;
        get_sol(network->current_block.target);

        if(!(sem_trywait(ganador))){
            Ganador = true;

            //Preparar votación
            sem_wait(mutex);
            network->current_block.solution = solution;
            network->current_block.pid_ganador = getpid();
            inicializar_votos(&network);
            sem_post(mutex);

            //TO DO: enviar USR2
            
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
            send_block();
            sem_post(mutex);

            //TO DO: preparar la siguiente ronda (bloque antiguo = actual, etc) y enviar USR1
            //TO DO:Enviar al Registrador por pipe
            /*TO DO:El proceso Minero finaliza cuando transcurre el número de segundos especificado
como argumento, o cuando recibe la se ̃nal SIGINT. En concreto, debe desregsitrarse del listado
en memoria compartida, y salir liberando todos los recursos. En el caso de que sea el  ́ultimo
minero de la red, debe eliminar adem ́as todos los recursos compartidos, y notificar al monitor
(si est ́a activo) el cierre del sistema envi ́andole un bloque de finalizaci ́on por cola de mensajes.
Por  ́ultimo, espera la finalizaci ́on de su proceso Registrador, quien debe detectar el cierre de la
tuber ́ıa, e imprime un mensaje de aviso en el caso de que no termine con el c ́odigo de salida
EXIT_SUCCESS*/

        } else {
            Ganador = false;
            //TO DO: Comprobar si ha llegado USR2. En caso contrario, esperar a que llegue

            //Votar
            sem_wait(mutex);
            if (pow_hash(network->current_block.solution) == network->current_block.target){
                mi_voto = 1;
            } else {
                mi_voto = 0;
            }
            network->votos[network_id] = mi_voto;
            sem_post(mutex);

            //TO DO: Wait for USR1
        }
    }
    
    exit(EXIT_SUCCESS);
}

//Todo el proceso de conexión a memoria y registro del minero está encapsulado aquí
void miner_startup(){
    int shm_mem;

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
    if(first_miner == true)
        init_network(&network);

    //El minero registra su pid en el sistema
    if (!registrar_pid(&network)){
        munmap(network,sizeof(struct Network));
        sem_post(mutex);
        sem_close(ganador);
        sem_close(mutex);
        exit(EXIT_FAILURE);
    }

    sem_post(mutex);
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
        return -1;
    }
    if (!(s = (struct Search_space *) malloc(sizeof(struct Search_space)*n_threads))){
        free(thread);
        printf("Error allocating memory");
        return -1;
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
    block->block_id = 0;
    block->target = 0;
    block->n_votos = 0;
    block->n_votos_pos = 0;
    block->initialised = true;
    block->flag = false;
    block->end = false;
}

void new_round_block (struct Block *new_block, struct Block old_block) {
    new_block->block_id = old_block.block_id + 1;
    new_block->target = old_block.solution;
    new_block->n_votos = 0;
    new_block->n_votos_pos = 0;
    new_block->initialised = true;
    new_block->flag = false;
    new_block->end = false;
}

void init_network (struct Network *network){
    int i;
    for(i = 0; i<MAX_MINEROS; i++){
        network->pids[i] = 0;
        network->votos[i] = 0;
        network->monederos[i].pid = 0;
        network->monederos[i].monedas = 0;
    }
    init_block(&network->current_block);
    init_block(&network->last_block);
    return;
}

int registrar_pid(struct Network *network){
    int i;
    for(i = 0; i<MAX_MINEROS; i++){
        if(network->pids[i] == 0){
            network->pids[i] = getpid();
            network->monederos[i].monedas = 0;
            network->monederos[i].pid = getpid();
            network_id = i;
            return 1;
        }
    }
    //El sistema está lleno
    return 0;
}

void inicializar_votos(struct Network *network){
    int i;
    for(i = 0; i<MAX_MINEROS; i++){
        network->votos[i] = 0;
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

void send_block(){
    mqd_t queue;
    struct timespec time = {2, 500};
    // Crear cola de mensajes
    queue = mq_open(MQ_NAME, O_WRONLY, S_IRUSR | S_IWUSR, &attributes);

    if (queue == (mqd_t)-1)
    {
        //There is no monitor or we couldn't access it, return
        mq_close(queue);
        return;
    }

    if (mq_timedsend(queue, (char *)&network->current_block, sizeof(struct Block), 1, &time) == -1)
    {
        perror("Send error");
        mq_close(queue);
        exit(EXIT_FAILURE);
    }
    mq_close(queue);
    return;
}
//TO DO: modularizar todas las funciones (separarlas en diferentes .c en función de su categoría)
//TO DO: tocar un poco el makefile para que funcione con este minero
//TO CHECK: cuando un minero abandona la network, debe poner su pid a 0 en la lista
//TO CHECK: pasamos como parámetro *network a algunas funciones, pero network ya es una variable global.
    //Estas dos networks, en la práctica, son iguales, pero puede que de problemas