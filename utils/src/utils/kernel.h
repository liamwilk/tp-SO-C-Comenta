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
    sem_t iniciar_planificador;
    sem_t detener_planificador;
    sem_t actualizar_planificador;
    bool continuar_planificador;
    t_kernel_sockets sockets;
    t_kernel_threads threads;
} t_kernel;

/**
 * Agrega un socket al kernel
 *
 * @param kernel El puntero al kernel.
 * @param type El tipo de socket agregar
 * @param socket El socket a agregar
 * @return El struct de sockets actualizado
 */
t_kernel_sockets kernel_sockets_agregar(t_kernel *kernel, KERNEL_SOCKETS type, int socket);

/**
 * @fn    kernel_inicializar
 * @brief Inicializa el kernel junto con todas sus configuraciones
 * @param config Instancia de module.config
 * @return Kernel: Struct de kernel
 */
t_kernel kernel_inicializar(t_config *config);

/**
 * @fn    log_kernel
 * @brief Log obligatorios de kernel junto con su configuracion
 * @param kernel Instancia de kernel (kernel_inicializar)
 * @param logger Instancia de t_log
 */
void kernel_log(t_kernel kernel, t_log *logger);

/**----PROCESOS Y ESTADOS----**/

/**
 * Inicializa los estados de las colas del kernel.
 *
 * Esta función inicializa los estados de las colas del kernel, que incluyen las colas new, ready, exec, block y exit.
 *
 * @param estados  Diagrama de 5 estados
 */
t_diagrama_estados kernel_inicializar_estados(t_diagrama_estados *estados);

/**
 * @brief Crea un nuevo proceso en el kernel.
 *
 * Esta función se encarga de crear un nuevo proceso en el kernel. Envia el struct t_kernel_memoria_proceso al modulo memoria
 *
 * @param kernel Un puntero a la estructura del kernel.
 * @param new  Un puntero a la cola de new.
 * @param logger Un puntero al logger.
 * @param instrucciones Una cadena de caracteres que contiene las instrucciones para el nuevo proceso.
 */
t_pcb *kernel_nuevo_proceso(t_kernel *kernel, t_diagrama_estados *estados, t_log *logger, char *instrucciones);

void kernel_enviar_pcb_cpu(t_kernel *kernel, t_pcb *pcb, KERNEL_SOCKETS cpu);

/** FUNCIONES DE CONSOLA**/

void kernel_finalizar(t_kernel *kernel);

#endif /* KERNEL_H */