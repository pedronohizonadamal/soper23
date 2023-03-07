#include "pow.h"
#include "monitor.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/wait.h>
#include <unistd.h>
#include <math.h>

long int solution;

typedef struct _Search_space Search_space;