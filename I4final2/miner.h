
#ifndef MINER_H
#define MINER_H

#include "miner_register.h"

#define MINER_NETWORK "miner_network"

#define INITIAL_TARGET 0
#define MUTEX "voting_shared_mutex"
#define GANADOR "voting_ganador_perdedor"
#define MAX_INTENTOS 2

struct Network {
    pid_t pids[MAX_MINEROS];
    int votos[MAX_MINEROS];
    struct Monedero monederos[MAX_MINEROS];
    struct Block last_block;
    struct Block current_block;
};

struct Search_space
{
  long int lower;
  long int upper;
  long int target;

};

void sigusr1_handler(int sig);
void sigusr2_handler(int sig);
void sigint_sigalarm_handler(int sig);

void get_sol(long target); 
void init_network (struct Network *net); 
int registrar_pid(struct Network *net); 
void miner_startup(); 
void inicializar_votos(struct Network *net); 
int check_votes(); 
void send_block(struct Block *block); 
void init_block (struct Block *block); 
void new_round_block (struct Block *current_block, struct Block *last_block);
void close_minero(int *pipe_);
void send_signals(int signal);
void miner_rush (int n_seconds, int *pipe_);
void *search(void *s);
bool init_pipe (int *pipe_);
void proceso_ganador (int *pipe_, int *intentos);
void proceso_perdedor (int *pipe_, int *mi_voto);

#endif