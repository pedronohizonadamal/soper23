
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

void get_sol(long target); //M
void init_network (struct Network *net); //M
int registrar_pid(struct Network *net); //M
void miner_startup(); //M
void inicializar_votos(struct Network *net); //M
int check_votes(); //M
void send_block(struct Block *block); //M
void init_block (struct Block *block); //M
void new_round_block (struct Block *current_block, struct Block *last_block); //M
void close_minero(int *pipe_); //MN
void send_signals(int signal); //M
void miner_rush (int n_seconds, int intentos, int mi_voto, long target, int *pipe_); //M
void *search(void *s);
bool init_pipe (int *pipe_);

#endif