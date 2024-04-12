#ifndef HILOS_H_
#define HILOS_H_
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <commons/log.h>
#include "modulos.h"

/**
 * @fn    hilo_crear_d
 * @brief Crea un hilo  detached
 * @param fn Funcion que ejecuta el hilo
 * @param args Argumentos del hilo
 * @param logger Logger para mostrar informacion
 */
pthread_t hilo_crear_d(void *fn, void *args, t_log *logger);

/**
 * @fn    hilo_handshake
 * @brief Realiza un handshake a un modulo a traves de un hilo tomando como struct a t_handshake
 */
void *hilo_handshake(void *args);

/**
 * @fn    hilo_crear_j
 * @brief Crea un hilo  con join
 * @param fn Funcion que ejecuta el hilo
 * @param args Argumentos del hilo
 */
void hilo_crear_j(void *fn, void *args, t_log *logger);

/**
 * @fn    hilo_exit
 * @brief Espera que todos los hilos en detached terminen su ejecucion para terminar el programa
 */
void hilo_exit();
#endif
