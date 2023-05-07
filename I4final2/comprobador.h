#ifndef COMPROBADOR_H
#define COMPROBADOR_H

#include "monitor_comprobador.h"

void sigint_handler(int sig);
void check_block(struct Block *block); //C

void insert_block(struct Block *block, struct MemoryQueue *mem_queue, int *queue_pos);
void proceso_monitor(sigset_t *block_signals, struct Block *block, int *completion_flag, int lag);
void proceso_comprobador(struct sigaction *sigint, sigset_t *block_signals, sigset_t *oldmask, int *completion_flag, struct Block *block, int lag, struct mq_attr *attributes);

#endif