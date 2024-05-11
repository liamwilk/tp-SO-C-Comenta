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
#include <semaphore.h>

t_kernel kernel;
t_log *logger;
t_config *config;
hilos_args args;
t_diagrama_estados estados;

void terminar_programa(int, t_log *, t_config *);

pthread_t thread_conectar_memoria;
pthread_t thread_conectar_cpu_dispatch;
pthread_t thread_conectar_cpu_interrupt;
pthread_t thread_esperar_entrada_salida;

void inicializar_args();
void inicializar_semaforos();

#endif /* MAIN_H_ */