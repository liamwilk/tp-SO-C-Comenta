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

t_kernel kernel;

t_log* logger;
t_config* config;


void terminar_programa(int, t_log*, t_config*);
int esperar_cliente(t_log*, int);
int recibir_operacion(int);
void *recibir_buffer(int*,int);


void* atender_io();

pthread_t io;

int socket_io;
int socket_server_kernel;
int socket_memoria;
int socket_cpu_interrupt;
int socket_cpu_dispatch;

#endif /* MAIN_H_ */