#ifndef PLANIFICACION_H_
#define PLANIFICACION_H_
#include "pthread.h"
#include <consola.h>

/**
 * Ejecuta el algoritmo de planificación a largo plazo.
 *
 * Esta función se encarga de ejecutar el algoritmo de planificación a largo plazo
 * en el kernel. Recibe como entrada un puntero a la estructura del kernel, un puntero
 * al diagrama de estados y un puntero al logger.
 *
 * @param hiloArgs   Un puntero a la estructura del kernel.
 * @param estados  Un puntero al diagrama de estados.
 * @param logger   Un puntero al logger.
 */
void planificacion_largo_plazo(hilos_args *hiloArgs);

/**
 * Ejecuta el algoritmo de planificación a corto plazo.
 *
 * Esta función se encarga de ejecutar el algoritmo de planificación a corto plazo
 * en el kernel. Recibe como entrada un puntero a la estructura del kernel, un puntero
 * al diagrama de estados y un puntero al logger.
 *
 * @param hiloArgs   Un puntero a la estructura del kernel.
 * @param estados  Un puntero al diagrama de estados.
 * @param logger   Un puntero al logger.
 */
void planificacion_corto_plazo(hilos_args *hiloArgs);

/**
 * @brief Función que implementa el algoritmo FIFO de planificación.
 *
 * Esta función recibe un puntero a una estructura de argumentos de hilo (hilos_args)
 * y realiza la planificación de los hilos utilizando el algoritmo FIFO.
 *
 * @param hiloArgs Puntero a la estructura de argumentos de hilo.
 */
void fifo(hilos_args *hiloArgs);

/**
 * @brief Función que implementa el algoritmo Round Robin de planificación.
 *
 * Esta función recibe un puntero a una estructura de argumentos de hilo (hilos_args)
 * y realiza la planificación de los hilos utilizando el algoritmo Round Robin.
 *
 * @param hiloArgs Puntero a la estructura de argumentos de hilo.
 */
void round_robin(hilos_args *hiloArgs);

/**
 * @brief Función que avisa al planificador para que realice la planificación.
 *
 * Esta función se utiliza para notificar al planificador que debe realizar la planificación
 * de los hilos en el sistema.
 */
void avisar_planificador();

typedef enum
{
    FIFO,
    RR,
    VRR
} t_algoritmo;

t_algoritmo determinar_algoritmo(hilos_args *args);

#endif /* PLANIFICACION_H_ */