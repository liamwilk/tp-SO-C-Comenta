
#ifndef KERNEL_STRUCTS_H_
#define KERNEL_STRUCTS_H_
#include "commons/collections/dictionary.h"
#include <semaphore.h>
#include "../procesos.h"

/*ENUMS y CONSTANTES*/

/**
 * @brief Enumeración que representa diferentes tipos de sockets del kernel.
 *
 * Esta enumeración define los diferentes tipos de sockets del kernel que se pueden utilizar en el sistema.
 * Cada tipo de socket representa un componente o funcionalidad diferente en el sistema.
 *
 * Los tipos de sockets disponibles son:
 * - MEMORIA: Representa un socket de memoria.
 * - CPU_DISPATCH: Representa un socket de despacho de CPU.
 * - CPU_INTERRUPT: Representa un socket de interrupción de CPU.
 * - SERVER: Representa un socket de servidor.
 * - ENTRADA_SALIDA_GENERIC: Representa un socket de entrada/salida genérico.
 * - ENTRADA_SALIDA_STDIN: Representa un socket de entrada estándar.
 * - ENTRADA_SALIDA_STDOUT: Representa un socket de salida estándar.
 * - ENTRADA_SALIDA_DIALFS: Representa un socket de entrada/salida DIALFS.
 */
typedef enum
{
    MEMORIA,
    CPU_DISPATCH,
    CPU_INTERRUPT,
    SERVER,
    ENTRADA_SALIDA_GENERIC,
    ENTRADA_SALIDA_STDIN,
    ENTRADA_SALIDA_STDOUT,
    ENTRADA_SALIDA_DIALFS,
    /**ESTO ES PARA PODER DETECTAR LAS OPERACIONES AL ENCOLAR IOs**/
    ENTRADA_SALIDA_DIALFS_CREATE,
    ENTRADA_SALIDA_DIALFS_WRITE,
    ENTRADA_SALIDA_DIALFS_READ,
    ENTRADA_SALIDA_DIALFS_DELETE,
    ENTRADA_SALIDA_DIALFS_TRUNCATE
} KERNEL_SOCKETS;

typedef struct
{
    int pid;
    int ocupado;
    int orden;
    int socket;
    bool identificado;
    bool valido;
    char *interfaz;
    KERNEL_SOCKETS tipo;
} t_kernel_entrada_salida;

typedef enum
{
    PROCESO_ESTADO,
    EJECUTAR_SCRIPT,
    INICIAR_PROCESO,
    MULTIPROGRAMACION,
    FINALIZAR_PROCESO,
    FINALIZAR_CONSOLA,
    DETENER_PLANIFICACION,
    INICIAR_PLANIFICACION,
    TOPE_ENUM_CONSOLA // siempre mantener este al final para saber el tamaño del enum
} t_consola_operacion;

/**
 * @brief Enumeración que representa los posibles motivos de finalización del kernel.
 */
typedef enum
{
    SUCCESS,
    INVALID_RESOURCE,
    SEGMENTATION_FAULT,
    INVALID_INTERFACE,
    OUT_OF_MEMORY,
    INTERRUPTED_BY_USER,
    EXECUTION_ERROR,
} KERNEL_MOTIVO_FINALIZACION;

typedef enum
{
    EXEC_READY,
    BLOCK_READY,
} t_transiciones_ready;

typedef struct t_kernel_sockets
{
    /*----MODULOS----*/
    int memoria;
    int cpu_dispatch;
    int cpu_interrupt;
    int server;
    /*----ENTRADA SALIDA----*/
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
    sem_t sistema_finalizar;
    sem_t log_lock;
    sem_t memoria_consola_finalizacion_proceso;
    bool detener_planificador;
    bool estado_planificador;
    t_kernel_sockets sockets;
    t_kernel_threads threads;
    pthread_mutex_t lock;
} t_kernel;

typedef struct
{
    timer_t timer;
    struct sigevent sev;
    struct itimerspec its;
    struct itimerspec quantum_restante; // Esto es el quantum restante del ultimo proceso en exec
    t_log *logger;
    t_kernel *kernel;
    t_diagrama_estados *estados;
    int kernel_orden_apagado;
    t_dictionary *recursos;
} hilos_args;

typedef struct
{
    hilos_args *args;
} timer_args_t;

typedef struct
{
    hilos_args *args;
    t_kernel_entrada_salida *entrada_salida;
} hilos_io_args;

typedef void (*t_funcion_kernel_ptr)(t_log *, t_op_code, hilos_args *, t_buffer *);
typedef void (*t_funcion_kernel_io_prt)(hilos_io_args *, char *, t_op_code, t_buffer *);

#endif /* KERNEL_STRUCTS_H_ */
