#include "monitor_comprobador.h"

bool init_semaphores(sem_t *mutex, sem_t *space, sem_t *blocks)
{
    if (sem_init(mutex, 1, 1) == -1) {
        perror("sem_init");
        shm_unlink(MONITOR_COMPROBADOR_SHARED_MEMORY);
        return false;
    }


    if (sem_init(space, 1, QUEUE_SIZE) == -1) {
        perror("sem_init");
        sem_destroy(mutex);
        shm_unlink(MONITOR_COMPROBADOR_SHARED_MEMORY);
        return false;
    }


    if (sem_init(blocks, 1, 0) == -1) {
        perror("sem_init");
        sem_destroy(mutex);
        sem_destroy(space);
        shm_unlink(MONITOR_COMPROBADOR_SHARED_MEMORY);
        return false;
    }

    return true;
}

