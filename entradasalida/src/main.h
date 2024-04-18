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
#include <utils/handshake.h>
#include <utils/modulos.h>
#include <utils/conexiones.h>
#include <utils/configs.h>
#include <utils/serial.h>

int socketMemoria;

t_log* logger_info;
t_log* logger_error;
t_log* logger_debug;
t_log* logger_warning;
t_log* logger_trace;

t_config* config;

t_entradasalida entradasalida;

pthread_t thread_conectar_kernel,thread_conectar_memoria;

void* conectar_kernel();
void* conectar_memoria();

int socket_memoria;
int socket_kernel;


#endif /* MAIN_H_ */