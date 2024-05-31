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

void fifo(hilos_args *hiloArgs);

void round_robin(hilos_args *hiloArgs);

void virtual_round_robin(hilos_args *hiloArgs);

typedef enum
{
    FIFO,
    RR,
    VRR
} t_algoritmo;

t_algoritmo determinar_algoritmo(hilos_args *args);

#endif /* PLANIFICACION_H_ */