#ifndef MAIN_H_
#define MAIN_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <commons/config.h>
#include <utils/modulos.h>
#include <utils/configs.h>
#include <utils/kernel.h>
#include "consola.h"
#include "hilos.h"
#include "pthread.h"

t_kernel kernel;
t_log *logger;
t_config *config;

diagrama_estados estados;

void terminar_programa(int, t_log *, t_config *);

pthread_t thread_atender_consola;
pthread_t thread_conectar_memoria;
pthread_t thread_atender_memoria;
pthread_t thread_conectar_cpu_dispatch;
pthread_t thread_conectar_cpu_interrupt;
pthread_t thread_atender_cpu_dispatch;
pthread_t thread_atender_cpu_interrupt;
pthread_t thread_esperar_entrada_salida;

// Cuando vale 0, es porque el Kernel ordeno a todos los modulos apagarse
int kernel_orden_apagado = 1;

#endif /* MAIN_H_ */