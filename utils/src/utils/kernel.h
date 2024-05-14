#ifndef KERNEL_H_
#define KERNEL_H_
#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <commons/config.h>
#include <readline/readline.h>
#include <utils/serial.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>
#include <limits.h>
#include <commons/collections/queue.h>
#include "procesos.h"
#include "conexiones.h"
#include <semaphore.h>

/*Estructura basica del kernel*/

typedef struct t_kernel_sockets
{
    int memoria;
    int cpu_dispatch;
    int cpu_interrupt;
    int server;
    int entrada_salida;
    int entrada_salida_generic;
    int entrada_salida_stdin;
    int entrada_salida_stdout;
    int entrada_salida_dialfs;
} t_kernel_sockets;

typedef enum KERNEL_SOCKETS
{
    MEMORIA,
    CPU_DISPATCH,
    CPU_INTERRUPT,
    SERVER,
    ENTRADA_SALIDA_GENERIC,
    ENTRADA_SALIDA_STDIN,
    ENTRADA_SALIDA_STDOUT,
    ENTRADA_SALIDA_DIALFS
} KERNEL_SOCKETS;

typedef struct
{
    pthread_t thread_atender_entrada_salida;
    pthread_t thread_atender_entrada_salida_generic;
    pthread_t thread_atender_entrada_salida_stdin;
    pthread_t thread_atender_entrada_salida_stdout;
    pthread_t thread_atender_entrada_salida_dialfs;
    pthread_t thread_atender_cpu_interrupt;
    pthread_t thread_atender_cpu_dispatch;
    pthread_t thread_atender_memoria;
    pthread_t thread_atender_consola;
    pthread_t thread_planificador;
} t_kernel_threads;

typedef struct t_kernel
{
    int puertoEscucha;
    char *ipMemoria;
    int puertoMemoria;
    char *ipCpu;
    int puertoCpuDispatch;
    int puertoCpuInterrupt;
    char *algoritmoPlanificador;
    int quantum;
    char *recursos;
    char *instanciasRecursos;
    int gradoMultiprogramacion;
    sem_t planificador_iniciar;
    sem_t sistema_finalizar;
    sem_t log_lock;
    sem_t memoria_consola_finalizacion_proceso;
    sem_t memoria_consola_nuevo_proceso;
    bool detener_planificador;
    t_kernel_sockets sockets;
    t_kernel_threads threads;
    pthread_mutex_t lock;
} t_kernel;

typedef struct
{
    t_log *logger;
    t_kernel *kernel;
    t_diagrama_estados *estados;
    int kernel_orden_apagado;
} hilos_args;

typedef struct
{
    hilos_args *args;
    int socket;
} hilos_io_args;

#endif /* KERNEL_H */