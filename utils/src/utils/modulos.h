#ifndef MODULOS_H_
#define MODULOS_H_
#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <commons/config.h>
#include "conexiones.h"
#include "hilos.h"

/*--------HANDSHAKING--------*/

/*Estructura basica del handshake*/
typedef struct t_handshake
{
    char *ipDestino;
    int puertoDestino;
    int socketDestino;
    char *modulo;
    t_log *logger;
} t_handshake;

/**
 * @fn    handshake_crear
 * @brief Realiza un handshake al modulo destino
 * @param config Instancia de `t_handshake` handshake
 * @param logger Instancia de `t_log`
 */
void handshake_crear(t_handshake *handshake, t_log *logger);

/*--------KERNEL--------*/

/*Estructura basica del kernel*/
typedef struct t_kernel
{
    char *puertoEscucha;
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

/**
 * @fn    kernel_handshake_cpu_dispatch
 * @brief Crea un handshake a memoria
 * @param kernel Instancia de kernel (kernel_inicializar)
 */
t_handshake kernel_handshake_memoria(t_kernel kernel, void *fn, t_log *logger);

/**
 * @fn    kernel_handshake_cpu_dispatch
 * @brief Crea un handshake a cpu dispatch
 * @param kernel Instancia de kernel (kernel_inicializar)
 */
t_handshake kernel_handshake_cpu_dispatch(t_kernel kernel, void *fn, t_log *logger);

/**
 * @fn    kernel_handshake_cpu_dispatch
 * @brief Crea un handshake a cpu dispatch
 * @param kernel Instancia de kernel (kernel_inicializar)
 */
t_handshake kernel_handshake_cpu_interrupt(t_kernel kernel, void *fn, t_log *logger);

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

/*--------Liberar memoria y al programa--------*/

/**
 * @fn    terminar_programa
 * @brief Recibe un array de conexiones y libera todos los recursos para terminar el programa
 * @param conexion Array de conexiones
 * @param logger Instancia de logger
 * @param config Instancia de config
 */
void terminar_programa(int conexion, t_log *logger, t_config *config);

/*--------Macros--------*/

#define CASTING(T)                                 \
    void casting_##T(struct T *arg, void **casted) \
    {                                              \
        *casted = (void *)arg;                     \
    }

#define DECASTING(T)                                   \
    void decasting_##T(struct T **arg, void *decasted) \
    {                                                  \
        *arg = (struct T *)decasted;                   \
    }

// Ejemplo de implementación
// struct dato{
//    int dato1;
//};
//
// CASTING(dato);
// DECASTING(dato);

// Ejemplo de uso
//
// pthread_t hilo1;
//
// struct dato v1 = {1 , 'g' , 1}; // parámetro que recibe la función
// void* c1 = NULL;  // puntero a void usado en el casting
// casting_dato(&v1 , &c1);
// pthread_create(&hilo1 , NULL , func1 , c1);
// pthread_detach(hilo1);
#endif