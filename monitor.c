#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include "pow.h"

int check(long int solution, long int target){

    return (int) pow_hash(solution) == target ? 1 : 0;
}