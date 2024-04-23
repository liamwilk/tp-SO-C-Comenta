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

t_cpu cpu;
t_log* logger;
t_config* config;

void* atender_kernel_dispatch();
void* atender_kernel_interrupt();
void* atender_memoria();

void* conectar_kernel_dispatch();
void* conectar_kernel_interrupt();
void* conectar_memoria();

int socket_kernel_dispatch;
int socket_kernel_interrupt;
int socket_memoria;
int socket_server_dispatch;
int socket_server_interrupt;

pthread_t thread_atender_kernel_dispatch,thread_atender_kernel_interrupt,thread_conectar_memoria,thread_atender_memoria,thread_conectar_kernel_dispatch,thread_conectar_kernel_interrupt;

// Cuando vale 0, es porque el Kernel ordeno a todos los modulos apagarse
int kernel_orden_apagado = 1;

#endif /* MAIN_H_ */