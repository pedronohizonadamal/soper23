#include "pow.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <math.h>

long int solution;

typedef struct _Search_space Search_space;