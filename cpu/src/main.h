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



pthread_t thread_atender_kernel_dispatch,thread_atender_kernel_interrupt,thread_conectar_memoria_dispatch,thread_conectar_memoria_interrupt;

t_cpu cpu;

t_log* logger_info;
t_log* logger_error;
t_log* logger_debug;
t_log* logger_warning;
t_log* logger_trace;

t_config* config;

void* atender_kernel_dispatch();
void* atender_kernel_interrupt();
void* conectar_memoria_dispatch();
void* conectar_memoria_interrupt();

int socket_kernel_dispatch;
int socket_kernel_interrupt;
int socket_memoria_dispatch;
int socket_memoria_interrupt;
int socket_server_dispatch;
int socket_server_interrupt;

#endif /* MAIN_H_ */