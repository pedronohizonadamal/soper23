#include "miner_rush.h"

// Imprime un blocque a fichero
void print_block (FILE *fp, struct Block block) {

    int i;

    fprintf(fp, "Id:       %04d\n",block.block_id);
    fprintf(fp, "Winner:   %d\n",block.pid_ganador);
    fprintf(fp, "Target:   %08ld\n",block.target);
    if (block.flag == true)
    {
        fprintf(fp, "Solution: %08ld (validated)\n", block.solution);
    }
    else
    {
        fprintf(fp, "Solution: %08ld (invalidated)\n", block.solution);
    }
    fprintf(fp, "Votes:    %d/%d\n",block.n_votos_pos,block.n_votos);
    fprintf(fp, "Wallets:");
    for(i = 0; i<MAX_MINEROS; i++){
        if(block.monederos[i].pid != 0){
            fprintf(fp, "  %d:%02d",block.monederos[i].pid,block.monederos[i].monedas);
        }
    }
    fprintf(fp, "\n\n");
}