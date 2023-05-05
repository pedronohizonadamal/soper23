#ifndef COMPROBADOR_H
#define COMPROBADOR_H

#include "monitor_comprobador.h"

mqd_t queue;

void sigint_handler(int sig);
void check_block(struct Block *block); //C

void insert_block(struct Block *block);

#endif