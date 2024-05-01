#ifndef MAIN_H_
#define MAIN_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>
#include <limits.h>
#include <utils/modulos.h>
#include <utils/handshake.h>
#include <utils/conexiones.h>
#include <utils/configs.h>
#include <utils/serial.h>
#include "consola.h"
#include <utils/kernel.h>

t_kernel kernel;
t_log *logger;
t_config *config;

diagrama_estados estados;

void terminar_programa(int, t_log *, t_config *);
int esperar_conexion(t_log *, int);

void *conectar_memoria();
void *conectar_cpu_dispatch();
void *conectar_cpu_interrupt();

void *atender_memoria();
void *atender_cpu_dispatch();
void *atender_cpu_interrupt();

void *esperar_entrada_salida();
void *atender_entrada_salida(void *);

void *atender_consola();

pthread_t thread_esperar_entrada_salida, thread_conectar_memoria, thread_conectar_cpu_dispatch, thread_conectar_cpu_interrupt, thread_atender_memoria, thread_atender_cpu_dispatch, thread_atender_cpu_interrupt, thread_atender_consola;

// Cuando vale 0, es porque el Kernel ordeno a todos los modulos apagarse
int kernel_orden_apagado = 1;

#endif /* MAIN_H_ */