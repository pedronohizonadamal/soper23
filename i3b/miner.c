#include "pow.h"
#include "monitor.h"
#include <mqueue.h>

int main(int argc, char **argv){
    int lag, rounds, i;
    long int target, solution;
    struct Block block;
    sem_t *empty, *fill;
    mqd_t queue;

    if(argc != 3){
        printf("Incorrect arguments. Usage: ./miner <ROUNDS> <LAG>\n");
        exit(EXIT_FAILURE);
    }

    rounds = atoi(argv[1]);
    lag = atoi(argv[2]);

    /* Semaphore init */
    if((empty = sem_open(MINER_CANDIDATE_QUEUE_IN, O_CREAT, S_IRUSR | S_IWUSR ,7))==SEM_FAILED){
        perror("Semaphore error");
        exit(EXIT_FAILURE);
    }
    if((fill = sem_open(MINER_CANDIDATE_QUEUE_OUT, O_CREAT, S_IRUSR | S_IWUSR ,0))==SEM_FAILED){
        perror("Semaphore error");
        sem_unlink(MINER_CANDIDATE_QUEUE_IN);
        exit(EXIT_FAILURE);
    }

    //Crear cola de mensajes
    queue = mq_open (MQ_NAME ,O_CREAT | O_WRONLY , S_IRUSR | S_IWUSR , &attributes);

    if (queue == (mqd_t) -1){
        fprintf (stderr,"Error opening the queue\n ");
        sem_unlink(MINER_CANDIDATE_QUEUE_IN);
        sem_unlink(MINER_CANDIDATE_QUEUE_OUT);
        exit(EXIT_FAILURE);
    }

    target = 0;
    block.end = false;
    for(i = 0; i<rounds; i++){
        block.target = target;
        solution = get_sol(target);
        block.solution = solution;

        //Enviar bloque por la cola de mensajes. El semáforo garantiza que no se pierda información.
        sem_wait(empty);
        //Mensaje
        if (mq_send ( queue , ( char *)&block , sizeof (block) , 1) == -1) {
        fprintf(stderr, "Error sending message\n ");
            sem_unlink(MINER_CANDIDATE_QUEUE_IN);
            sem_unlink(MINER_CANDIDATE_QUEUE_OUT);
            mq_close ( queue );
            exit(EXIT_FAILURE);
        }
        sem_post(fill);

        sleep(lag);
        target = solution;
    }
    //Enviar mensaje final
    block.end = true;
    //Enviar por la cola de mensajes
    sem_unlink(MINER_CANDIDATE_QUEUE_IN);
    sem_unlink(MINER_CANDIDATE_QUEUE_OUT);
    mq_close ( queue );
    mq_unlink ( MQ_NAME );
    exit(EXIT_SUCCESS);
}

long get_sol(long target){
    int flag = 0;
    long int i = 0, current_sol;
    while(!flag){
        //Final alcanzado
        if (i == POW_LIMIT)
            flag = 1;
        //Comprobar i
        current_sol = pow_hash(i);
        if (current_sol == target)
            flag = 1;
    }
    return i;
}