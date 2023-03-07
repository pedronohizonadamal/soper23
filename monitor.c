#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include "pow.h"

int check(long int solution, long int target){

    // argv[0] is the target
    // argv[1] is the solution

    return (int) pow_hash(solution) == target ? 1 : 0;
}