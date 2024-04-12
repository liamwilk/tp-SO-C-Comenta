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

// macros para funciones de casting usando structs
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