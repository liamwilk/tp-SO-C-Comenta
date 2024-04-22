#ifndef MAIN_H_
#define MAIN_H_

#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include<sys/socket.h>
#include<netdb.h>
#include<unistd.h>
#include<pthread.h>
#include<limits.h>
#include <utils/modulos.h>
#include <utils/handshake.h>
#include <utils/conexiones.h>
#include <utils/configs.h>
#include <utils/serial.h>

t_kernel kernel;

t_log* logger_info;
t_log* logger_error;
t_log* logger_debug;
t_log* logger_warning;
t_log* logger_trace;

t_config* config;


void terminar_programa(int, t_log*, t_config*);
int esperar_cliente(t_log*, int);
int recibir_operacion(int);
void *recibir_buffer(int*,int);

void* conectar_memoria();
void* conectar_cpu_dispatch();
void* conectar_cpu_interrupt();
void* atender_io(void* args);
void* procesar_io();

pthread_t thread_atender_io,thread_conectar_memoria,thread_conectar_cpu_dispatch,thread_conectar_cpu_interrupt;

int socket_server_kernel;
int socket_memoria;
int socket_cpu_interrupt;
int socket_cpu_dispatch;

#endif /* MAIN_H_ */