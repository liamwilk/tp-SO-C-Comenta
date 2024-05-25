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

typedef enum
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

typedef enum
{
    SUCCESS,
    INVALID_RESOURCE,
    INVALID_INTERFACE,
    OUT_OF_MEMORY,
    INTERRUPTED_BY_USER
} KERNEL_MOTIVO_FINALIZACION;

typedef struct
{
    int pid;
    int ocupado;
    int orden;
    int socket;
    char *interfaz;
    KERNEL_SOCKETS tipo;
} t_kernel_entrada_salida;

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
    int id_entrada_salida;
    t_list *list_entrada_salida;
    t_dictionary *dictionary_entrada_salida;
} t_kernel_sockets;

typedef struct
{
    pthread_t thread_atender_entrada_salida;
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
    sem_t planificador_hilo;
    sem_t sistema_finalizar;
    sem_t log_lock;
    sem_t memoria_consola_finalizacion_proceso;
    sem_t memoria_consola_nuevo_proceso;
    bool detener_planificador;
    bool estado_planificador;
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
    t_kernel_entrada_salida *entrada_salida;
} hilos_io_args;

/**
 * @brief Formatea y registra un mensaje genérico con un número variable de argumentos.
 *
 * Esta función toma un número variable de argumentos y los formatea en un mensaje utilizando la cadena de formato proporcionada.
 * Luego, registra el mensaje formateado en el nivel de registro especificado.
 *
 * @param args Un puntero a la estructura hilos_args.
 * @param nivel El nivel de registro en el que se registrará el mensaje.
 * @param mensaje La cadena de formato para el mensaje.
 * @param ... Los argumentos variables que se formatearán en el mensaje.
 */
void kernel_log_generic(hilos_args *args, t_log_level nivel, const char *mensaje, ...) __attribute__((format(printf, 3, 4)));

void kernel_log_generic_rl(hilos_args *args, t_log_level nivel, const char *mensaje);

/**
 * Inicializa los estados de las colas del kernel.
 *
 * Esta función inicializa los estados de las colas del kernel, que incluyen las colas new, ready, exec, block y exit.
 *
 * @param estados  Diagrama de 5 estados
 */
t_diagrama_estados kernel_inicializar_estados(t_diagrama_estados *estados);

/**
 * @brief Desaloja un proceso del kernel.
 *
 * Esta función se encarga de desalojar un proceso del kernel, actualizando el diagrama de estados,
 * Se utiliza principalmente en ROUND ROBIN o VIRTUAL ROUND ROBIN
 *
 * @param kernel_hilos_args Son los argumentos que recibe cada hilo al momento de ejecutarse
 */
void kernel_desalojar_proceso(hilos_args *kernel_hilos_args);

/**
 * Transiciona un PCB del estado EXEC al estado READY en el diagrama de estados del kernel.
 *
 * @param kernel_hilos_args Son los argumentos que recibe cada hilo al momento de ejecutarse
 */
t_pcb *kernel_transicion_exec_ready(hilos_args *kernel_hilos_args);

/**
 * Transiciona un PCB del estado READY al estado EXEC en el kernel.
 *
 * Esta función toma como parámetros un diagrama de estados y un kernel, y transiciona un Bloque de Control de Proceso (PCB)
 * del estado READY al estado EXEC en el kernel. Retorna un puntero al PCB actualizado.
 *
 * @param kernel_hilos_args Son los argumentos que recibe cada hilo al momento de ejecutarse
 */
t_pcb *kernel_transicion_ready_exec(hilos_args *kernel_hilos_args);

/**
 * Transiciona un proceso del estado bloqueado al estado listo en el kernel.
 *
 * Esta función toma como parámetros un diagrama de estados y un logger. Extrae un proceso del estado bloqueado,
 * lo coloca en el estado listo y devuelve el proceso. Si no hay procesos en el estado bloqueado, devuelve NULL.
 *
 * @param kernel_hilos_args Son los argumentos que recibe cada hilo al momento de ejecutarse
 * @param modulo
 * @param unidad
 */
t_pcb *kernel_transicion_block_ready(hilos_io_args *io_args, char *modulo, t_entrada_salida_kernel_unidad_de_trabajo *unidad);

/**
 * @brief Función de transición para ejecutar un bloque en el kernel.
 *
 * Esta función realiza la transición del estado exec al estado "block" y devuelve el siguiente PCB (Bloque de Control de Proceso)
 * a ser ejecutado según el diagrama de estados y el logger proporcionados.
 *
 * @param kernel_hilos_args Son los argumentos que recibe cada hilo al momento de ejecutarse
 */
t_pcb *kernel_transicion_exec_block(hilos_args *kernel_hilo_args);

/**
 * @brief Realiza la transición del estado EXEC al estado EXIT en el kernel.
 *
 * Esta función actualiza el diagrama de estados y registra la transición en el logger.
 *
 * @param kernel_hilos_args Son los argumentos que recibe cada hilo al momento de ejecutarse
 */
t_pcb *kernel_transicion_exec_exit(hilos_args *kernel_hilo_args);

/**
 * @brief Función de transición para ejecutar un bloque en el kernel.
 *
 * Esta función realiza la transición del estado new al estado "ready" y devuelve el  PCB (Bloque de Control de Proceso)
 *
 * @param kernel_hilos_args Son los argumentos que recibe cada hilo al momento de ejecutarse
 */
t_pcb *kernel_transicion_new_ready(hilos_args *kernel_hilo_args);

void kernel_finalizar(hilos_args *args);

t_kernel_entrada_salida *entrada_salida_buscar_interfaz(hilos_args *args, char *interfaz);

void log_ready(hilos_args *kernel_hilos_args);

bool kernel_finalizar_proceso(hilos_args *kernel_hilos_args, uint32_t pid, KERNEL_MOTIVO_FINALIZACION MOTIVO);

void kernel_revisar_paquete(t_paquete *paquete, hilos_args *args, char *modulo);

#endif /* KERNEL_H */