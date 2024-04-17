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

t_log* logger;
t_config* config;

t_memoria memoria;


typedef struct 
{
    t_log* logger;
	char* ip;
    int puerto;
	int socket;
	char* mensaje;
} t_paqueteCliente;


void* atender_cpu_dispatch();
void* atender_cpu_interrupt();
void* atender_kernel();
void* atender_io();

pthread_t thread_atender_cpu_dispatch,thread_atender_cpu_interrupt,thread_atender_kernel,thread_atender_io;

int socket_cpu_dispatch;
int socket_cpu_interrupt;
int socket_kernel;
int socket_io;
int socket_server_memoria;

#endif /* MAIN_H_ */