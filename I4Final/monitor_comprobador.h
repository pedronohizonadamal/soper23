#ifndef MONITOR_COMPROBADOR_H
#define MONITOR_COMPROBADOR_H

#include "miner_rush.h"

#define INITIAL_TARGET 0
#define MUTEX "voting_shared_mutex"
#define GANADOR "voting_ganador_perdedor"
#define MAX_INTENTOS 2

#define MINER_CANDIDATE_QUEUE_IN "miner_candidate_multiplex_in"
#define MINER_CANDIDATE_QUEUE_OUT "miner_candidate_multiplex_out"
#define MONITOR_COMPROBADOR_SHARED_MEMORY "monitor_shmem"

bool init_semaphores(sem_t *mutex, sem_t *space, sem_t *blocks);

#endif