#include "monitor.h"
#include "pow.h"


int receive_block(struct Block *block, mqd_t queue)
{
    if (mq_receive(queue, (char *)block, sizeof(*block), NULL) == -1)
    {
        perror("MQ_receive_error");
        return 0;
    }
    return 1;
}


