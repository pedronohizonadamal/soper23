#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/mman.h>

#define PIDS_FILE "pids.txt"
#define VOTOS_FILE "votos.txt"


static int *esperando = NULL; // nº de procesos Votante en espera
static int *candidato = NULL; // PID del proceso candidato

// semáforos
sem_t mutex1, mutex2, mutex3, mutex4, lsem, esem;
sem_t sem1, sem2;

int contlect = 0; // contador de lectores
int contesc = 0;  // contador de escritores


// Puntero a función de entrada/salida
typedef int (*io_function)();

// Funciones de tipo entrada/salida
int escribir_pid();
int contar_registros();
int emitir_voto();

/**
  @brief Escribe el PID del proceso en un fichero
  @returns un entero, 0 si todo ha ido correctamente y -1 en caso contrario
  */
int escribir_pid(int pid)
{

    FILE *f = NULL;

    f = fopen(PIDS_FILE, "a+");
    if (f == NULL)
        return -1;

    fprintf(f, "%d\n", getpid());

    /**
     El semáforo mutex4 sirve para esperar a cerrar un archivo abierto antes de finalizar el programa
     */
    sem_wait(&mutex4);
    fclose(f);
    sem_post(&mutex4);

    return 0;
}

/**
  @brief Cuenta los registros de un fichero, utilizamos esta función para identificar si ya se han
  creado todos los procesos, es decir, si el sistema ya está listo
  @returns un entero, número de registros leidos o -1 en caso de error
  */
int contar_registros()
{

    int i = 0;

    FILE *f = NULL;
    char buffer[512];

    f = fopen(PIDS_FILE, "r");
    if (f == NULL)
        return -1;

    // printf("Estoy contanto!! PID: %d\n", getpid());

    while (fscanf(f, "%[^\n] ", buffer) != EOF)
    {
        i++;
    };

    // printf("LEIDAS %d LÍNEAS\n", i);

    /**
     El semáforo mutex4 sirve para esperar a cerrar un archivo abierto antes de finalizar el programa
     */
    sem_wait(&mutex4);
    fclose(f);
    sem_post(&mutex4);

    return i;
}


/**
  @brief Cuenta el número de registros en el fichero de votos, utilizamos esta función para identificar 
  si ya han votado todos los procesos Votante
  @returns un entero, número de registros leidos o -1 en caso de error
  */
int contar_votos()
{

    int i = 0;

    FILE *f = NULL;
    char buffer[512];

    f = fopen(VOTOS_FILE, "r");
    if (f == NULL)
        return -1;

    // printf("Estoy contanto!! PID: %d\n", getpid());

    while (fscanf(f, "%[^\n] ", buffer) != EOF)
    {
        i++;
    };

    // printf("LEIDAS %d LÍNEAS\n", i);

    /**
     El semáforo mutex4 sirve para esperar a cerrar un archivo abierto antes de finalizar el programa
     */
    sem_wait(&mutex4);
    fclose(f);
    sem_post(&mutex4);

    return i;
}


/**
  @brief Comprueba el resultado de la votación
  @returns un entero, número de votos a favor o -1 en caso de error
  */
int contabilizar_votos()
{

    int i = 0;

    FILE *f = NULL;
    int pid;
    char voto;

    f = fopen(VOTOS_FILE, "r");
    if (f == NULL)
        return -1;

    printf("Candidate %d => [ ", getpid());

    while (fscanf(f, "%d %c\n", &pid, &voto) != EOF)
    {
        printf("%c ", voto);
        if(voto == 'Y')
            i++;
    };

    printf("]");

    /**
     El semáforo mutex4 sirve para esperar a cerrar un archivo abierto antes de finalizar el programa
     */
    sem_wait(&mutex4);
    fclose(f);
    sem_post(&mutex4);

    return i;
}


/**
  @brief Permite leer un fichero sin que se produzcan problemas de concurrencia (problema de los
  lectores-escritores dando preferencia a los escritores)
  @param leer es un puntero a la función de lectura que se quiere utilizar
  @returns un entero, devuelve el resultado de leer()
  */
int reader(int (*leer)())
{

    int n = 0;

    sem_wait(&mutex3);
    sem_wait(&lsem);
    sem_wait(&mutex1);
    contlect++;
    if (contlect == 1)
        sem_wait(&esem);
    sem_post(&mutex1);
    sem_post(&lsem);
    sem_post(&mutex3);

    n = leer();

    sem_wait(&mutex1);
    contlect--;
    if (contlect == 0)
        sem_post(&esem);
    sem_post(&mutex1);

    return n;
}

/**
  @brief Permite escribir en un fichero sin que se produzcan problemas de concurrencia (problema de los
  lectores-escritores dando preferencia a los escritores)
  @param escribir es un puntero a la función de escritura que se quiere utilizar
  @returns un entero, devuelve el resultado de escribir()
  */
int writer(int (*escribir)())
{

    int n = 0;

    sem_wait(&mutex2);
    contesc++;
    if (contesc == 1)
        sem_wait(&lsem);
    sem_post(&mutex2);
    sem_wait(&esem);

    n = escribir();

    sem_post(&esem);
    sem_wait(&mutex2);
    contesc--;
    if (contesc == 0)
        sem_post(&lsem);
    sem_post(&mutex2);

    return n;
}

/**
  @brief Función que permite destruir los semáforos antes de finalizar el programa
  */
void destruir_sems()
{
    sem_destroy(&mutex1);
    sem_destroy(&mutex2);
    sem_destroy(&mutex3);
    sem_destroy(&mutex4);
    sem_destroy(&esem);
    sem_destroy(&lsem);
    sem_destroy(&sem1);
    sem_destroy(&sem2);
}

/**
 @brief Inicialización de todos los semáforos
 */
void init_semaphores() {
    sem_init(&mutex1, 1, 1);
    sem_init(&mutex2, 1, 1);
    sem_init(&mutex3, 1, 1);
    sem_init(&mutex4, 1, 1);
    sem_init(&esem, 1, 1);
    sem_init(&lsem, 1, 1);
    sem_init(&sem1, 1, 0);
    sem_init(&sem2, 1, 0);
}

/**
  @brief Permite enviar una señal a todos los procesos hijo leyendo el PID de cada uno
  en el fichero correspondiente
  @param sig es la señal que se quiere enviar a todos los procesos hijo
  */
void send_to_pid(int sig)
{

    FILE *f = NULL;
    pid_t pid;

    f = fopen(PIDS_FILE, "r");
    if (f == NULL)
        exit(EXIT_FAILURE);

    while (fscanf(f, "%d ", &pid) != EOF)
    {
        if (kill(pid, sig) == -1)
        {
            exit(EXIT_FAILURE);
        }
    };

    /**
     El semáforo mutex4 sirve para esperar a cerrar un archivo abierto antes de finalizar el programa
     */
    sem_wait(&mutex4);
    fclose(f);
    sem_post(&mutex4);
}

/**
  @brief Como para enviar la señal necesitamos realizar una lectura del fichero, utilizamos
  la estructura de la función reader() con el método send_to_pid() visto antes
  @param sig es la señal que se quiere enviar a todos los procesos hijo
  */
void send_signal(int sig)
{

    sem_wait(&mutex3);
    sem_wait(&lsem);
    sem_wait(&mutex1);
    contlect++;
    if (contlect == 1)
        sem_wait(&esem);
    sem_post(&mutex1);
    sem_post(&lsem);
    sem_post(&mutex3);

    send_to_pid(sig);

    sem_wait(&mutex1);
    contlect--;
    if (contlect == 0)
        sem_post(&esem);
    sem_post(&mutex1);
}


/**
 @brief Escribe el voto de un proceso en el fichero correspondiente
 @returns un entero, 0 si todo ha ido correctamente y -1 en caso contrario
 */
int emitir_voto() {
    
    FILE *f = NULL;
    pid_t pid = getpid(), ppid = getppid();


    f = fopen(VOTOS_FILE, "a+");
    if (f == NULL)
        return -1;

    if((pid+rand())%2) {
        fprintf(f, "%d %c\n", getpid(), 'N');
    } else {
        fprintf(f, "%d %c\n", getpid(), 'Y');
    }

    /**
     El semáforo mutex4 sirve para esperar a cerrar un archivo abierto antes de finalizar el programa
     */
    sem_wait(&mutex4);
    fclose(f);
    sem_post(&mutex4);

    return 0;

}

/**
  @brief Handler para la señal SIGINT que se encarga de reenviar la señal a los procesos
  Votante y termina el programa Principal mostrando antes un mensaje
  */
void handler_SIGINT()
{
    printf("Finishing by signal\n");
    send_signal(SIGINT);
    destruir_sems();
    exit(EXIT_SUCCESS);
}

/**
  @brief Handler para la señal SIGALRM que se encarga de enviar la señal SIGINT a los
  procesos Votante y termina el programa Principal mostrando antes un mensaje
  */
void handler_SIGALRM()
{
    printf("Finishing by alarm\n");
    send_signal(SIGINT);
    destruir_sems();
    exit(EXIT_SUCCESS);
}

/**
  @brief Handler para la señal SIGALRM que se encarga de enviar la señal SIGINT a los
  procesos Votante y termina el programa Principal mostrando antes un mensaje
  */
void handler_SIGUSR1()
{
    //printf("Se activa el proceso Votante con PID: %d\n", getpid());
}

/**
  @brief Handler para la señal SIGUSR2 que convierte un proceso Votante en Candidato
  procesos Votante y termina el programa Principal mostrando antes un mensaje
  */
void handler_SIGUSR2(int signum, siginfo_t *siginfo, void *context)
{

      
    /* Mostramos toda la información necesaria 
    printf ("[%d] Recibida %d (%s) de %d (UID: %d) ¿Error? %d \n",
        getpid(),
        signum,
        strsignal(signum),
        siginfo->si_pid,
        siginfo->si_uid,
        siginfo->si_errno);
    */

}

int main(int argc, char *argv[])
{

    int n_procs, n_secs, i;
    pid_t pid = 0, wpid = -1;
    FILE *f = NULL;


    srand(time(NULL));

    int *esperando = (int*)mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE,
               MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    *esperando = 0;

    int *candidato = (int*)mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE,
        MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    *candidato = 0;



    /**
     Si ya existe el fichero, se borra su contenido.
     Si no existe, se crea el fichero.
     */
    f = fopen(PIDS_FILE, "w");
    if (f == NULL)
        exit(EXIT_FAILURE);
    fclose(f);

    /**
     Si ya existe el fichero, se borra su contenido.
     Si no existe, se crea el fichero.
     */
    f = fopen(VOTOS_FILE, "w");
    if (f == NULL)
        exit(EXIT_FAILURE);
    fclose(f);

    struct sigaction act1, act2, act3, act4;
    sigset_t sigset;

    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s -<N_PROCS> <N_SECS>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    n_procs = (pid_t)atoi(argv[1]);
    n_secs = atoi(argv[2]);

    init_semaphores();
    

    /**
     El proceso Principal crea n_procs Votante
     */
    for (i = 0; i < n_procs; i++)
    {

        pid = fork();

        if (pid == -1)
        { // Si hay error, se aborta la operación
            perror("fork");
            destruir_sems();
            exit(EXIT_FAILURE);
        }
        else if (pid == 0)
        {
            break;
        }
    }

    if (pid == 0)
    {

        /**
         Lógica del proceso Votante
         */
        sigemptyset(&(act3.sa_mask));
        act3.sa_flags = 0;
        act3.sa_handler = handler_SIGUSR1;
        sigfillset(&act3.sa_mask);
        if (sigaction(SIGUSR1, &act3, NULL) < 0)
        {
            perror("sigaction");
            exit(EXIT_FAILURE);
        }


        sigemptyset(&(act4.sa_mask));
        act4.sa_flags = SA_SIGINFO;
        act4.sa_sigaction = handler_SIGUSR2;
        sigfillset(&act4.sa_mask);
        if (sigaction(SIGUSR2, &act4, NULL) < 0)
        {
            perror("sigaction");
            exit(EXIT_FAILURE);
        }


        sigfillset(&sigset);
        sigdelset(&sigset, SIGUSR1);
        sigdelset(&sigset, SIGINT);

        writer(escribir_pid);


        /**
          El proceso Votante espera a la señal SIGUSR1
          */
        // printf("Comienza la espera del proceso Votante con PID: %d\n", getpid());
        sem_wait(&mutex4);
        *esperando = *esperando + 1;
        sem_post(&mutex4);
        sigsuspend(&sigset); // pause SIGUSR1

        while(1){

            /**
             Proceso Candidato
            */ 
            if(*candidato == 0) {
                *candidato = getpid();


                //printf("CANDIDATO: %d, PID: %d\n", *candidato, getpid());

                while (reader(contar_votos) < n_procs - 1)
                {
                    //printf("n_votos = %d\n", reader(contar_votos));
                    //printf("Todavía no.\n");
                }

                if(reader(contabilizar_votos) > (n_procs-1)/2){
                    printf(" => Accepted\n");
                } else {
                    printf(" => Rejected\n");
                }

            }

            /**
             Procesos Votante distintos del Candidato
            */ 
            if((int) getpid() != *candidato) {

                writer(emitir_voto);

                sem_wait(&mutex4);
                *esperando = *esperando + 1;
                sem_post(&mutex4);
                sigsuspend(&sigset); // pause SIGUSR1

            }

        }

        exit(EXIT_SUCCESS);

    }
    else
    {

        /**
         Lógica del proceso Principal
         */

        sigemptyset(&(act1.sa_mask));
        act1.sa_flags = 0;
        act1.sa_handler = handler_SIGALRM;
        sigfillset(&act1.sa_mask);
        if (sigaction(SIGALRM, &act1, NULL) < 0)
        {
            perror("sigaction");
            exit(EXIT_FAILURE);
        }

        sigemptyset(&(act2.sa_mask));
        act2.sa_flags = 0;
        act2.sa_handler = handler_SIGINT;
        sigfillset(&act2.sa_mask);
        if (sigaction(SIGINT, &act2, NULL) < 0)
        {
            perror("sigaction");
            exit(EXIT_FAILURE);
        }


        if (alarm(n_secs))
        {
            fprintf(stderr, "There is a previously established alarm\n");
        }

        /**
         Una vez listo el sistema, se envía la señal SIGUSR1 a los procesos Votante (hijo)
         */

        while(*esperando < n_procs){
            //printf("n_esperando = %d\n", *esperando);
        };
        //printf("El Principal envía la señal SIGUSR1\n");
        send_signal(SIGUSR1);

        while(1){
            while(*esperando < n_procs - 1){
                //printf("n_esperando = %d\n", *esperando);
            };
            //printf("El Principal envía la señal SIGUSR1\n");
            send_signal(SIGUSR1);

            *esperando = 0;
        }

        // printf("Soy el padre: %d\n", getpid());

    }

    destruir_sems();
    munmap(esperando, sizeof(int));
    munmap(candidato, sizeof(int));

    /**
     Esperamos a que todos los procesos Votante terminen
     */
    for (int i = 0; i < n_procs; i++) {
        if ((wpid = wait(NULL)) >= 0) {
            // printf("Proceso %d terminado\n", wpid);
        }
    }
    
    exit(EXIT_SUCCESS);

}
