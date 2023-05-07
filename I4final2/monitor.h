#ifndef MONITOR_H
#define MONITOR_H

#include "monitor_comprobador.h"

int receive_block(struct Block *block, mqd_t queue);

#endif
