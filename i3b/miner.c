#include "pow.h"
#include "monitor.h"
#include <mqueue.h>

int main(int argc, char **argv)
{
    int lag, rounds, i;
    long int target, solution;
    struct Block block;
    mqd_t queue;

    if (argc != 3)
    {
        printf("Incorrect arguments. Usage: ./miner <ROUNDS> <LAG>\n");
        exit(EXIT_FAILURE);
    }

    rounds = atoi(argv[1]);
    lag = atoi(argv[2]);

    // Crear cola de mensajes
    queue = mq_open(MQ_NAME, O_CREAT | O_EXCL | O_WRONLY, S_IRUSR | S_IWUSR, &attributes);

    if (queue == (mqd_t)-1)
    {
        perror("Queue error");
        exit(EXIT_FAILURE);
    }

    target = 0;
    block.end = false;
    printf("[%d] Generating blocks...\n", (int)getpid());
    for (i = 0; i < rounds; i++)
    {
        block.target = target;
        solution = get_sol(target);
        block.solution = solution;

        // Mensaje
        if (mq_send(queue, (char *)&block, sizeof(block), 1) == -1)
        {
            perror("Send error");
            mq_close(queue);
            exit(EXIT_FAILURE);
        }

        sleep(lag);
        target = solution;
    }

    // Enviar mensaje final
    block.end = true;
    // Enviar por la cola de mensajes
    if (mq_send(queue, (char *)&block, sizeof(block), 1) == -1)
    {
        perror("Send error");
        mq_close(queue);
        exit(EXIT_FAILURE);
    }

    // End
    printf("[%d] Finishing\n", (int)getpid());
    mq_close(queue);
    /*mq_unlink(MQ_NAME);*/
    exit(EXIT_SUCCESS);
}

long get_sol(long target)
{
    int flag = 0;
    long int i = 0, current_sol;
    while (!flag)
    {
        // Final alcanzado
        if (i == POW_LIMIT)
            flag = 1;
        // Comprobar i
        current_sol = pow_hash(i);
        if (current_sol == target)
            flag = 1;
        i++;
    }
    return i-1;
}
