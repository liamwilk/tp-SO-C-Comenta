#ifndef MODULOS_H_
#define MODULOS_H_
#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <commons/config.h>
#include<readline/readline.h>
#include<sys/socket.h>
#include<netdb.h>
#include<unistd.h>
#include<pthread.h>
#include<limits.h>

/*--------KERNEL--------*/

/*Estructura basica del kernel*/
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
} t_kernel;

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


/*--------CPU--------*/

/*Estructura basica de la CPU*/

typedef struct t_cpu
{
    int puertoEscuchaDispatch;
    int puertoEscuchaInterrupt;
    char *ipMemoria;
    int puertoMemoria;
    int cantidadEntradasTlb;
    char *algoritmoTlb;
} t_cpu;

/**
 * @fn    cpu_inicializar
 * @brief Inicializa el cpu junto con todas sus configuraciones
 * @param config Instancia de module.config
 * @return config
 */
t_cpu cpu_inicializar(t_config *config);

/**
 * @fn    cpu_log
 * @brief Logs necesarios para cpu
 * @param config Instancia de module.config
 * @return t_cpu
 */
void cpu_log(t_cpu cpu, t_log *logger);


typedef struct t_entradasalida
{
    char *ipMemoria;
    int puertoMemoria;
    char *ipKernel;
    int puertoKernel;
    char *tipoInterfaz;
    int tiempoUnidadDeTrabajo;
    char *pathBaseDialFs;
    int blockSize;
    int blockCount;
} t_entradasalida;

/**
 * @fn    entradasalida_inicializar
 * @brief Inicializa la entrada-salida junto con todas sus configuraciones
 * @param config Instancia de module.config
 * @return config
 */
t_entradasalida entradasalida_inicializar(t_config *config);

/**
 * @fn    entradasalida_log
 * @brief Logs necesarios para entrada-salida
 * @param config Instancia de module.config
 */
void entradasalida_log(t_entradasalida entradasalida, t_log *logger);

/*--------Memoria--------*/
typedef struct t_memoria
{
    int tamMemoria, tamPagina, retardoRespuesta;
    char *pathInstrucciones;
    char *puertoEscucha;
} t_memoria;

/**
 * @fn    memoria_inicializar
 * @brief Inicializa la memoria junto con todas sus configuraciones
 * @param config Instancia de module.config
 * @return config
 */
t_memoria memoria_inicializar(t_config *config);

/**
 * @fn    memoria_log
 * @brief Logs necesarios para memoria
 * @param config Instancia de module.config
 */
void memoria_log(t_memoria memoria, t_log *logger);


/*--------Serializacion y sockets--------*/

typedef enum
{
    MENSAJE,
    PAQUETE
} op_code;

typedef struct
{
    int size;
    void *stream;
} t_buffer;

typedef struct
{
    op_code codigo_operacion;
    t_buffer *buffer;
} t_paquete;

t_paquete *crear_paquete(void);

/**
 * @fn    serializar_paquete
 * @brief Implementacion de la serializacion de un paquete (strings)
 * @param paquete Paquete con buffer y su op_code
 * @param bytes
 */
void *serializar_paquete(t_paquete *paquete, int bytes);

/**
 * @fn    enviar_mensaje
 * @brief Envia un mensaje `mensaje` al modulo conectado
 * @param mensaje Paquete con buffer y su op_code
 * @param socket_cliente
 */
void enviar_mensaje(char *mensaje, int socket_cliente);

/**
 *
 * @fn    crear_buffer
 * @brief Crea un buffer y lo agrega al paquete
 */
void crear_buffer(t_paquete *paquete);

/**
 *
 * @fn    agregar_a_paquete
 * @brief Agrega al paquete un `valor` de tipo generico
 * @param paquete Paquete de datos
 * @param valor Agrega el tipo generico
 * @param tamanio
 */
void agregar_a_paquete(t_paquete *paquete, void *valor, int tamanio);

/**
 *
 * @fn    enviar_paquete
 * @brief Dado un socket envia el paquete al modulo destino
 * @param paquete Paquete de datos
 * @param socket_cliente
 */
void enviar_paquete(t_paquete *paquete, int socket_cliente);

/**
 *
 * @fn    eliminar_paquete
 * @brief Elimina el paquete para no generar memory leak
 * @param paquete Paquete de datos
 */
void eliminar_paquete(t_paquete *paquete);

/**
 *
 * @fn    recibir_buffer
 * @brief Elimina el paquete para no generar memory leak
 * @param paquete Paquete de datos
 */
void *recibir_buffer(int *size, int socket_cliente);

/**
 *
 * @fn    recibir_mensaje
 * @brief Invoca la funcion recv y recibe los datos desde el socket
 * @param socket_cliente
 */
void recibir_mensaje(t_log *logger, int socket_cliente);

/**
 *
 * @fn    recibir_operacion
 * @brief Invoca la funcion recv y recibe el codigo de operacion desde el socket
 * @param socket_cliente
 */
int recibir_operacion(int socket_cliente);




#endif /* MODULOS_H_ */
